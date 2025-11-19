import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import CONF_ENTITY_CATEGORY, ENTITY_CATEGORY_CONFIG, CONF_ID

from .. import EmersonR48Component, emerson_r48_ns, CONF_EMERSON_R48_ID

EmersonR48Button = emerson_r48_ns.class_(
    "EmersonR48Button", button.Button, cg.Component
)
WalkInButton = emerson_r48_ns.class_(
    "WalkInButton", button.Button, cg.Component
)
RestartOvervoltageButton = emerson_r48_ns.class_(
    "RestartOvervoltageButton", button.Button, cg.Component
)

CONF_SET_OFFLINE_VALUES = "set_offline_values"
CONF_WALK_IN_ENABLE = "walk_in_enable"
CONF_WALK_IN_DISABLE = "walk_in_disable"
CONF_RESTART_OVERVOLTAGE_ENABLE = "restart_overvoltage_enable"
CONF_RESTART_OVERVOLTAGE_DISABLE = "restart_overvoltage_disable"
CONF_WALK_IN_TIME = "walk_in_time"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_EMERSON_R48_ID): cv.use_id(EmersonR48Component),
            cv.Optional(CONF_SET_OFFLINE_VALUES): button.button_schema(
                EmersonR48Button,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ).extend(
                {
                    cv.Required('name'): cv.string_strict
                }
            ),
            cv.Optional(CONF_WALK_IN_ENABLE): button.button_schema(
                WalkInButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ).extend(
                {
                    cv.Required('name'): cv.string_strict,
                    cv.Optional(CONF_WALK_IN_TIME, default=0.0): cv.float_range(min=0.0),
                }
            ),
            cv.Optional(CONF_WALK_IN_DISABLE): button.button_schema(
                WalkInButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ).extend(
                {
                    cv.Required('name'): cv.string_strict
                }
            ),
            cv.Optional(CONF_RESTART_OVERVOLTAGE_ENABLE): button.button_schema(
                RestartOvervoltageButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ).extend(
                {
                    cv.Required('name'): cv.string_strict
                }
            ),
            cv.Optional(CONF_RESTART_OVERVOLTAGE_DISABLE): button.button_schema(
                RestartOvervoltageButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ).extend(
                {
                    cv.Required('name'): cv.string_strict
                }
            ),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_EMERSON_R48_ID])
    
    if CONF_SET_OFFLINE_VALUES in config:
        conf = config[CONF_SET_OFFLINE_VALUES]
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await button.register_button(var, conf)
        cg.add(var.set_parent(hub))
    
    if CONF_WALK_IN_ENABLE in config:
        conf = config[CONF_WALK_IN_ENABLE]
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await button.register_button(var, conf)
        cg.add(var.set_parent(hub))
        cg.add(var.set_enable(True))
        cg.add(var.set_time(conf[CONF_WALK_IN_TIME]))
    
    if CONF_WALK_IN_DISABLE in config:
        conf = config[CONF_WALK_IN_DISABLE]
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await button.register_button(var, conf)
        cg.add(var.set_parent(hub))
        cg.add(var.set_enable(False))
        cg.add(var.set_time(0.0))  # FIXED: Added missing time setting for disable
    
    if CONF_RESTART_OVERVOLTAGE_ENABLE in config:
        conf = config[CONF_RESTART_OVERVOLTAGE_ENABLE]
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await button.register_button(var, conf)
        cg.add(var.set_parent(hub))
        cg.add(var.set_enable(True))
    
    if CONF_RESTART_OVERVOLTAGE_DISABLE in config:
        conf = config[CONF_RESTART_OVERVOLTAGE_DISABLE]
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await button.register_button(var, conf)
        cg.add(var.set_parent(hub))
        cg.add(var.set_enable(False))
