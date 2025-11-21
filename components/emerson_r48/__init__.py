import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import canbus
from esphome.const import (
    CONF_ID,
    CONF_UPDATE_INTERVAL,
)

DEPENDENCIES = ["canbus"]
AUTO_LOAD = ["sensor", "number", "switch", "button"]

CONF_CANBUS_ID = "canbus_id"
CONF_EMERSON_R48_ID = "emerson_r48_id"  # ADD THIS LINE
CONF_OFFLINE_VOLTAGE = "offline_voltage"
CONF_OFFLINE_CURRENT_PERCENT = "offline_current_percent"

emerson_r48_ns = cg.esphome_ns.namespace("emerson_r48")
EmersonR48Component = emerson_r48_ns.class_("EmersonR48Component", cg.Component)

# Also add backward compatibility alias
EmersonR48 = EmersonR48Component

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(EmersonR48Component),
        cv.Required(CONF_CANBUS_ID): cv.use_id(canbus.Canbus),
        cv.Optional(CONF_UPDATE_INTERVAL, default="5s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_OFFLINE_VOLTAGE, default=53.1): cv.float_range(min=41.0, max=58.5),
        cv.Optional(CONF_OFFLINE_CURRENT_PERCENT, default=50.0): cv.float_range(min=10.0, max=121.0),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    canbus_var = await cg.get_variable(config[CONF_CANBUS_ID])
    cg.add(var.set_canbus(canbus_var))
    
    cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))
    cg.add(var.set_offline_voltage(config[CONF_OFFLINE_VOLTAGE]))
    cg.add(var.set_offline_current_percent(config[CONF_OFFLINE_CURRENT_PERCENT]))
