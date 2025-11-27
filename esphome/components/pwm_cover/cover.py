from esphome import pins
import esphome.codegen as cg
from esphome.components import cover, output, binary_sensor
from esphome.components.rotary_encoder.sensor import RotaryEncoderSensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_DEFAULT_TRANSITION_LENGTH,
    CONF_OPEN_ENDSTOP,
    CONF_CLOSE_ENDSTOP,
)

CONF_ROTARY_ENCODER = "rotary_encoder"
CONF_OPENING_PIN = "opening_pin"
CONF_CLOSING_PIN = "closing_pin"
CONF_PWM_OUTPUT = "pwm_output"
CONF_MAX_ENCODER_VALUE = "max_encoder_value"
CONF_ESTIMATED_BREAKING_PWM_LEVEL = "estimated_breaking_pwm_level"
CONF_MOVEMENT_CONTROLLER = "pwm_cover_movement_controller"
CONF_MIN_SPEED_COEF = "min_speed_coef"

pwm_cover_ns = cg.esphome_ns.namespace("pwm_cover")
PwmCover = pwm_cover_ns.class_("PwmCover", cover.Cover, cg.Component)

PwmCoverMovementController = pwm_cover_ns.class_("PwmCoverMovementController")

CONFIG_SCHEMA = cv.All(
    cover.cover_schema(PwmCover)
    .extend(
        {
            cv.GenerateID(CONF_MOVEMENT_CONTROLLER): cv.declare_id(PwmCoverMovementController),
            cv.Required(CONF_OPENING_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_CLOSING_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_PWM_OUTPUT): cv.use_id(output.FloatOutput),
            cv.Required(CONF_OPEN_ENDSTOP): cv.use_id(binary_sensor.BinarySensor),
            cv.Required(CONF_CLOSE_ENDSTOP): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_ROTARY_ENCODER): cv.use_id(RotaryEncoderSensor),
            cv.Optional(CONF_ESTIMATED_BREAKING_PWM_LEVEL): cv.zero_to_one_float,
            cv.Optional(CONF_MIN_SPEED_COEF): cv.zero_to_one_float,
            cv.Optional(
                CONF_DEFAULT_TRANSITION_LENGTH,
                default="4s"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(
                CONF_MAX_ENCODER_VALUE,
                default=7100
            ): cv.positive_not_null_float,
        }
    )
)

async def to_code(config):
    mc = cg.new_Pvariable(config[CONF_MOVEMENT_CONTROLLER])

    pwm_output = await cg.get_variable(config[CONF_PWM_OUTPUT])
    cg.add(mc.set_pwm_output(pwm_output))

    if CONF_ROTARY_ENCODER in config:
        encoder = await cg.get_variable(config[CONF_ROTARY_ENCODER])
        cg.add(mc.set_encoder_sensor(encoder))

    if (
        default_transition_length := config.get(CONF_DEFAULT_TRANSITION_LENGTH)
    ) is not None:
        cg.add(mc.set_default_transition_length(default_transition_length))

    if (
        max_encoder_value := config.get(CONF_MAX_ENCODER_VALUE)
    ) is not None:
        cg.add(mc.set_max_encoder_value(max_encoder_value))

    if CONF_ESTIMATED_BREAKING_PWM_LEVEL in config:
        cg.add(mc.set_estimated_breaking_pwm_level(config[CONF_ESTIMATED_BREAKING_PWM_LEVEL]))

    if CONF_MIN_SPEED_COEF in config:
        cg.add(mc.set_min_speed_coef(config[CONF_MIN_SPEED_COEF]))

    var = await cover.new_cover(config, mc)
    await cg.register_component(var, config)

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

