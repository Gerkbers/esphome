from esphome import pins
import esphome.codegen as cg
from esphome.components import cover, output, binary_sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_OPEN_ENDSTOP,
    CONF_CLOSE_ENDSTOP,
)

CONF_OPENING_PIN = "opening_pin"
CONF_CLOSING_PIN = "closing_pin"
CONF_PWM_OUTPUT = "pwm_output"
CONF_ACCELERATION_TRANSITION_LENGTH = "acceleration_transition_length"
CONF_DECELERATION_TRANSITION_LENGTH = "deceleration_transition_length"
CONF_CLOSING_TIME = "closing_time"
CONF_OPENING_TIME = "opening_time"

pwm_endstop_cover_ns = cg.esphome_ns.namespace("pwm_endstop_cover")
PwmEndstopCover = pwm_endstop_cover_ns.class_("PwmEndstopCover", cover.Cover, cg.Component)

CONFIG_SCHEMA = cv.All(
    cover.cover_schema(PwmEndstopCover)
    .extend(
        {
            cv.Required(CONF_OPENING_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_CLOSING_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_PWM_OUTPUT): cv.use_id(output.FloatOutput),
            cv.Required(CONF_OPEN_ENDSTOP): cv.use_id(binary_sensor.BinarySensor),
            cv.Required(CONF_CLOSE_ENDSTOP): cv.use_id(binary_sensor.BinarySensor),
            cv.Required(CONF_OPENING_TIME): cv.positive_time_period_milliseconds,
            cv.Required(CONF_CLOSING_TIME): cv.positive_time_period_milliseconds,
            cv.Optional(
                CONF_ACCELERATION_TRANSITION_LENGTH,
                default="4s"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(
                CONF_DECELERATION_TRANSITION_LENGTH,
                default="4s"
            ): cv.positive_time_period_milliseconds,
        }
    )
)

async def to_code(config):
    var = await cover.new_cover(config)
    await cg.register_component(var, config)

    pwm_output = await cg.get_variable(config[CONF_PWM_OUTPUT])
    cg.add(var.set_pwm_output(pwm_output))

    if (
        acceleration_transition_length := config.get(CONF_ACCELERATION_TRANSITION_LENGTH)
    ) is not None:
        cg.add(var.set_acceleration_transition_length(acceleration_transition_length))

    if (
        deceleration_transition_length := config.get(CONF_DECELERATION_TRANSITION_LENGTH)
    ) is not None:
        cg.add(var.set_deceleration_transition_length(deceleration_transition_length))

    cg.add(var.set_opening_time(config[CONF_OPENING_TIME]))
    cg.add(var.set_closing_time(config[CONF_CLOSING_TIME]))

    cg.add_define("CONF_OPENING_PIN")
    opening_pin = await cg.gpio_pin_expression(config[CONF_OPENING_PIN])
    cg.add(var.set_opening_pin(opening_pin))

    cg.add_define("CONF_CLOSING_PIN")
    closing_pin = await cg.gpio_pin_expression(config[CONF_CLOSING_PIN])
    cg.add(var.set_closing_pin(closing_pin))

    bin = await cg.get_variable(config[CONF_OPEN_ENDSTOP])
    cg.add(var.set_open_endstop(bin))

    bin = await cg.get_variable(config[CONF_CLOSE_ENDSTOP])
    cg.add(var.set_close_endstop(bin))
