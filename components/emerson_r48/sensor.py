import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_POWER,
    STATE_CLASS_MEASUREMENT,
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_WATT,
    UNIT_PERCENT,
)
from . import emerson_r48_ns, EmersonR48Component, CONF_CANBUS_ID

DEPENDENCIES = ['emerson_r48']

CONF_OUTPUT_VOLTAGE = 'output_voltage'
CONF_OUTPUT_CURRENT = 'output_current'
CONF_MAX_OUTPUT_CURRENT = 'max_output_current'
CONF_OUTPUT_TEMP = 'output_temp'
CONF_INPUT_VOLTAGE = 'input_voltage'
CONF_OUTPUT_POWER = 'output_power'
CONF_MAX_CURRENT_AMPERES = 'max_current_amperes'
CONF_POWER_HEADROOM = 'power_headroom'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(EmersonR48Component),
    cv.Optional(CONF_OUTPUT_VOLTAGE): sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_VOLTAGE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_OUTPUT_CURRENT): sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_CURRENT,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_MAX_OUTPUT_CURRENT): sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_OUTPUT_TEMP): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_INPUT_VOLTAGE): sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_VOLTAGE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_OUTPUT_POWER): sensor.sensor_schema(
        unit_of_measurement=UNIT_WATT,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_POWER,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_MAX_CURRENT_AMPERES): sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_CURRENT,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_POWER_HEADROOM): sensor.sensor_schema(
        unit_of_measurement=UNIT_WATT,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_POWER,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])
    
    if CONF_OUTPUT_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_OUTPUT_VOLTAGE])
        cg.add(parent.set_output_voltage_sensor(sens))
    
    if CONF_OUTPUT_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_OUTPUT_CURRENT])
        cg.add(parent.set_output_current_sensor(sens))
    
    if CONF_MAX_OUTPUT_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_MAX_OUTPUT_CURRENT])
        cg.add(parent.set_max_output_current_sensor(sens))
    
    if CONF_OUTPUT_TEMP in config:
        sens = await sensor.new_sensor(config[CONF_OUTPUT_TEMP])
        cg.add(parent.set_output_temp_sensor(sens))
    
    if CONF_INPUT_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_INPUT_VOLTAGE])
        cg.add(parent.set_input_voltage_sensor(sens))
    
    if CONF_OUTPUT_POWER in config:
        sens = await sensor.new_sensor(config[CONF_OUTPUT_POWER])
        cg.add(parent.set_output_power_sensor(sens))
    
    if CONF_MAX_CURRENT_AMPERES in config:
        sens = await sensor.new_sensor(config[CONF_MAX_CURRENT_AMPERES])
        cg.add(parent.set_max_current_amperes_sensor(sens))
    
    if CONF_POWER_HEADROOM in config:
        sens = await sensor.new_sensor(config[CONF_POWER_HEADROOM])
        cg.add(parent.set_power_headroom_sensor(sens))
