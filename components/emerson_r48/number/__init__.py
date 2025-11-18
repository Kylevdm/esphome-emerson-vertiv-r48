import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import (
    UNIT_VOLT,
    UNIT_PERCENT,  # Add this
    CONF_ID,
    CONF_ICON,
    CONF_UNIT_OF_MEASUREMENT,
    CONF_MODE,
    CONF_ENTITY_CATEGORY,
    ICON_FLASH,
    ICON_CURRENT_AC,
    CONF_MIN_VALUE,
    CONF_MAX_VALUE,
    CONF_STEP,
    UNIT_AMPERE,
    ENTITY_CATEGORY_NONE,
)

from .. import EmersonR48Component, emerson_r48_ns, CONF_EMERSON_R48_ID

CONF_OUTPUT_VOLTAGE = "output_voltage"
CONF_MAX_OUTPUT_CURRENT = "max_output_current"
CONF_MAX_INPUT_CURRENT = "max_input_current"

EmersonR48Number = emerson_r48_ns.class_(
    "EmersonR48Number", number.Number, cg.Component
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_EMERSON_R48_ID): cv.use_id(EmersonR48Component),
            cv.Optional(CONF_OUTPUT_VOLTAGE): number.number_schema(
                EmersonR48Number,
                icon=ICON_FLASH,
                unit_of_measurement=UNIT_VOLT,
                entity_category=ENTITY_CATEGORY_NONE,
            ).extend(
                {
                    cv.Optional(CONF_MIN_VALUE, default=41): cv.float_,
                    cv.Optional(CONF_MAX_VALUE, default=58.5): cv.float_,
                    cv.Optional(CONF_STEP, default=0.1): cv.float_,
                    cv.Optional(CONF_MODE, default="BOX"): cv.enum(
                        number.NUMBER_MODES, upper=True
                    ),
                }
            ),
            cv.Optional(CONF_MAX_OUTPUT_CURRENT): number.number_schema(
                EmersonR48Number,
                icon=ICON_FLASH,
                unit_of_measurement=UNIT_PERCENT,
                entity_category=ENTITY_CATEGORY_NONE,
            ).extend(
                {
                    cv.Optional(CONF_MIN_VALUE, default=10): cv.float_,
                    cv.Optional(CONF_MAX_VALUE, default=121): cv.float_,
                    cv.Optional(CONF_STEP, default=1): cv.float_,
                    cv.Optional(CONF_MODE, default="BOX"): cv.enum(
                        number.NUMBER_MODES, upper=True
                    ),
                }
            ),
            cv.Optional(CONF_MAX_INPUT_CURRENT): number.number_schema(
                EmersonR48Number,
                icon=ICON_CURRENT_AC,
                unit_of_measurement=UNIT_AMPERE,
                entity_category=ENTITY_CATEGORY_NONE,
            ).extend(
                {
                    cv.Optional(CONF_MIN_VALUE, default=0): cv.float_,
                    cv.Optional(CONF_MAX_VALUE, default=20): cv.float_,
                    cv.Optional(CONF_STEP, default=0.1): cv.float_,
                    cv.Optional(CONF_MODE, default="BOX"): cv.enum(
                        number.NUMBER_MODES, upper=True
                    ),
                }
            ),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_EMERSON_R48_ID])
    if CONF_OUTPUT_VOLTAGE in config:
        conf = config[CONF_OUTPUT_VOLTAGE]
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await number.register_number(
            var,
            conf,
            min_value=conf[CONF_MIN_VALUE],
            max_value=conf[CONF_MAX_VALUE],
            step=conf[CONF_STEP],
        )
        cg.add(getattr(hub, "set_output_voltage_number")(var))
        cg.add(var.set_parent(hub, 0x0))
    if CONF_MAX_OUTPUT_CURRENT in config:
        conf = config[CONF_MAX_OUTPUT_CURRENT]
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await number.register_number(
            var,
            conf,
            min_value=conf[CONF_MIN_VALUE],
            max_value=conf[CONF_MAX_VALUE],
            step=conf[CONF_STEP],
        )
        cg.add(getattr(hub, "set_max_output_current_number")(var))
        cg.add(var.set_parent(hub, 0x3))
    if CONF_MAX_INPUT_CURRENT in config:
        conf = config[CONF_MAX_INPUT_CURRENT]
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await number.register_number(
            var,
            conf,
            min_value=conf[CONF_MIN_VALUE],
            max_value=conf[CONF_MAX_VALUE],
            step=conf[CONF_STEP],
        )
        cg.add(getattr(hub, "set_max_input_current_number")(var))
        cg.add(var.set_parent(hub, 0x4))