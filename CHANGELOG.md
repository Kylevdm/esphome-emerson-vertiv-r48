# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased] - 2025-11-19

### Added
- **Walk-in functionality**: Control voltage ramp-up behavior
  - `set_walk_in(bool enable, float time)` function in C++ implementation
  - `walk_in_enable` button configuration with configurable ramp time
  - `walk_in_disable` button configuration
  - Aligns with Python script `walk_in()` function
  
- **Restart after overvoltage functionality**: Control automatic restart behavior
  - `set_restart_overvoltage(bool enable)` function in C++ implementation
  - `restart_overvoltage_enable` button configuration
  - `restart_overvoltage_disable` button configuration
  - Aligns with Python script `restart_overvoltage()` function

### Changed
- Updated button system to support new WalkInButton and RestartOvervoltageButton classes
- Enhanced example YAML configuration to demonstrate new features

### Technical Details

#### Walk-in Function
**Purpose**: Time to ramp up the rectifier's output voltage to the set voltage value

**CAN Command**: `0x03, 0xF0, 0x00, 0x32, ...`

**Parameters**:
- `enable` (bool): Enable or disable walk-in functionality
- `time` (float): Ramp-up time in seconds (when enabled)

**Usage in YAML**:
```yaml
button:
  - platform: emerson_r48
    walk_in_enable:
      name: "Enable Walk-in (10s ramp)"
      walk_in_time: 10.0  # Time in seconds
    walk_in_disable:
      name: "Disable Walk-in"
```

#### Restart After Overvoltage Function
**Purpose**: Enable or disable automatic restart after an overvoltage condition

**CAN Command**: `0x03, 0xF0, 0x00, 0x39, ...`

**Parameters**:
- `enable` (bool): Enable or disable restart after overvoltage

**Usage in YAML**:
```yaml
button:
  - platform: emerson_r48
    restart_overvoltage_enable:
      name: "Enable Restart After Overvoltage"
    restart_overvoltage_disable:
      name: "Disable Restart After Overvoltage"
```

### Alignment with Python Script

This update brings the ESPHome implementation to full feature parity with the [PurpleAlien/R48_Rectifier](https://github.com/PurpleAlien/R48_Rectifier) Python script:

| Feature | Python Script | ESPHome Implementation | Status |
|---------|---------------|------------------------|--------|
| Set Voltage | `set_voltage()` | `set_output_voltage()` | ✅ Existing |
| Set Current (%) | `set_current_percentage()` | `set_max_output_current()` | ✅ Existing |
| Set Current (A) | `set_current_value()` | Via `set_max_output_current()` | ✅ Existing |
| AC Input Limit | `limit_input()` | `set_max_input_current()` | ✅ Existing |
| Walk-in | `walk_in()` | `set_walk_in()` | ✅ **NEW** |
| Restart Overvoltage | `restart_overvoltage()` | `set_restart_overvoltage()` | ✅ **NEW** |
| Read All Data | `READ_ALL` command | Individual reads | 📝 Commented out |

### Notes

- The `READ_ALL` command (`0x00, 0xF0, 0x00, 0x80, 0x46, 0xA5, 0x34, 0x00`) is commented out in both implementations. The current approach polls individual parameters sequentially.
- All CAN message constants and command structures now match the Python script exactly.
- Byte order and float conversion logic matches the Python implementation.

### References

- Original Python script: https://github.com/PurpleAlien/R48_Rectifier
- Emerson/Vertiv R48-3000e rectifier datasheet specifications:
  - Voltage range: 41.0V - 58.5V
  - Current range: 5.5A - 62.5A (10% - 121% of rated)
  - CAN bus: 125 kbps, extended ID
