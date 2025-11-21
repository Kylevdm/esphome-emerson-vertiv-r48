import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID

from .. import emerson_r48_ns, EmersonR48Component, CONF_EMERSON_R48_ID

DEPENDENCIES = ['emerson_r48']

EmersonR48Number = emerson_r48_ns.class_('EmersonR48Number', number.Number, cg.Component)

CONF_OUTPUT_VOLTAGE = 'output_voltage'
CONF_MAX_OUTPUT_CURRENT = 'max_output_current'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_EMERSON_R48_ID): cv.use_id(EmersonR48Component),
    cv.Optional(CONF_OUTPUT_VOLTAGE): number.number_schema(
        EmersonR48Number,
        icon="mdi:flash"
    ).extend(cv.COMPONENT_SCHEMA),
    cv.Optional(CONF_MAX_OUTPUT_CURRENT): number.number_schema(
        EmersonR48Number,
        icon="mdi:current-dc"
    ).extend(cv.COMPONENT_SCHEMA),
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_EMERSON_R48_ID])
    
    if CONF_OUTPUT_VOLTAGE in config:
        conf = config[CONF_OUTPUT_VOLTAGE]
        var = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(
            var, conf,
            min_value=41.0,
            max_value=58.5,
            step=0.01,
        )
        await cg.register_component(var, conf)
        cg.add(var.set_parent(parent))
        cg.add(var.set_type(0))  # 0 = voltage
    
    if CONF_MAX_OUTPUT_CURRENT in config:
        conf = config[CONF_MAX_OUTPUT_CURRENT]
        var = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(
            var, conf,
            min_value=10.0,
            max_value=121.0,
            step=1.0,
        )
        await cg.register_component(var, conf)
        cg.add(var.set_parent(parent))
        cg.add(var.set_type(1))  # 1 = current
