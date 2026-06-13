# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

An [ESPHome](https://esphome.io) **external component** that controls an Emerson / Vertiv R48 rectifier (a ~3kW telecom DC power supply, e.g. R48-3000e3) over a CAN bus. It exposes the rectifier's telemetry and controls as Home Assistant entities. There is no standalone application here — the code is compiled *by ESPHome* into firmware for an ESP8266 or ESP32 that is wired to the rectifier through an MCP2515 SPI-to-CAN transceiver.

The CAN protocol was reverse-engineered; the canonical reference is the comments throughout `components/emerson_r48/emerson_r48.cpp` and the upstream Python project https://github.com/PurpleAlien/R48_Rectifier.

## Build / validate / run

There is no `make`, `cmake`, or test suite. The only meaningful build is asking ESPHome to validate and compile a config that references this component:

```bash
# Validate a config (fast, catches Python codegen + schema errors)
esphome config emerson_r48_example_esp32_mcp2515.yaml

# Full compile via ESPHome / PlatformIO (slow; pulls toolchain + ESPHome core)
esphome compile emerson_r48_example_esp32_mcp2515.yaml

# Build and flash to a connected device
esphome run emerson_r48_example_esp32_mcp2515.yaml
```

To test local changes, point `external_components.source` at the working tree instead of GitHub:

```yaml
external_components:
  - source:
      type: local
      path: components
```

The two example YAMLs are the closest thing to integration tests — `emerson_r48_example.yaml` (ESP8266 / esp01_1m, MQTT) and `emerson_r48_example_esp32_mcp2515.yaml` (ESP32 / esp-idf, native API). Note they reference *different* GitHub forks in `external_components.source`; update that line when validating against the current repo.

## Architecture

This follows ESPHome's **hub + platform** pattern. Understanding the split between the Python codegen layer and the C++ runtime is essential.

### Hub (`components/emerson_r48/`)

`EmersonR48Component` (`emerson_r48.h` / `.cpp`) is the central `PollingComponent`. It is the only thing that touches the CAN bus. It:

- Holds a pointer to the ESPHome `canbus::Canbus` and registers an `add_callback` lambda in `setup()` so every received frame flows into `on_frame()`.
- In `update()` (default 1s), walks a `cnt` state machine: each tick requests one telemetry value (output V, output A, current limit, temp, supply V) and on the 6th tick re-sends the control word + sync/keepalive frames.
- Owns nullable pointers to *every possible* sensor and number entity. The platform files inject these via `set_*_sensor()` / `set_*_number()`. `publish_sensor_state_` / `publish_number_state_` null-check before publishing, so unconfigured entities are simply skipped.
- Holds the four boolean control flags (`acOff_`, `dcOff_`, `fanFull_`, `flashLed_`) that switches mutate.

### Platforms attach to the hub

Each ESPHome platform is a subdirectory (or top-level file) with a Python `to_code` that resolves the hub via `CONF_EMERSON_R48_ID` and wires entities into it:

- **`sensor.py`** — read-only telemetry. Maps config keys to `set_<key>_sensor()` on the hub.
- **`number/`** — writable setpoints (output voltage, max output current %, max input current A). Each `EmersonR48Number` is given a `functionCode_` via `set_parent(hub, code)`; `control()` dispatches on that code to the matching `parent_->set_*()` method.
- **`switch/`** — the four control toggles. `EmersonR48Switch::write_state` sets the corresponding flag on the hub, recomputes the packed control byte, and calls `parent_->set_control()`.
- **`button/`** — single `set_offline_values` button that calls `parent_->set_offline_values()` to persist the current voltage/current setpoints to the rectifier's non-volatile config.

The `functionCode` passed to `set_parent()` is the dispatch key inside the C++ `control()`/`write_state()` switch — these integer codes in the Python `to_code` (`0x0`, `0x3`, `0x4`, etc.) must stay in sync with the `static const ... FUNCTION` constants in the corresponding `.cpp`.

### CAN protocol specifics (in `emerson_r48.cpp`)

- Distinct extended CAN IDs for requesting data, receiving data, setting values, control, and sync/keepalive — see the `CAN_ID_*` constants.
- Data frames carry a parameter selector in `data[3]`; `on_frame()` switches on it to route the value to the right sensor.
- Floats are transmitted as their raw 4-byte IEEE-754 big-endian representation (`float_to_bytearray` / the `memcpy`+shift pattern), **not** scaled integers.
- **Online vs offline writes:** setters take an `offline` bool. Online writes (default) are temporary — the rectifier reverts after ~30s, so they must be repeated. Offline writes (parameter byte `0x24`/`0x19`) persist. The `set_offline_values` button is how a user makes the current setpoints permanent.
- Output current is set as a **percentage** of the rated 62.5A (= 121%), not as amps.

### `components/mcp2515/`

A vendored copy of ESPHome's stock MCP2515 CAN driver, bundled so users get a compatible version (notably the `add_callback` hook the hub relies on) without depending on a specific ESPHome release. Treat it as a third-party dependency: don't refactor it for style, only touch it for genuine compatibility fixes.

## Conventions and gotchas

- All component C++ lives in `namespace esphome::emerson_r48`. Note the closing comment in `emerson_r48.cpp` mistakenly reads `namespace huawei_r4850` (this project was derived from the esphome-huawei-r4850 component) — harmless, but don't propagate it.
- The files contain large blocks of commented-out code: unused sensors (input frequency/power/temp, efficiency, output power), alternative request strategies, and Python-reference snippets for protocol commands not yet implemented (`walk_in`, `restart_overvoltage`, etc.). These are intentional documentation/scaffolding for future protocol work — leave them unless asked to clean up.
- `switch/empty_switch.*` and `switch/switch.py` define an unrelated `empty_switch` skeleton platform that is not part of the R48 feature set; ignore it for R48 work.
- When adding a new readable value: add the parameter constant + `on_frame()` case in `emerson_r48.cpp`, the setter/pointer in `emerson_r48.h`, and the config key + schema in `sensor.py`. When adding a new writable value: add the `set_*()` method on the hub, a new `FUNCTION` code, and wire it in the platform's `__init__.py` `to_code`.
