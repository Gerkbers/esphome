import esphome.codegen as cg
from esphome.components import cover, uart

curtains_ns = cg.esphome_ns.namespace("curtains_cover")
CurtainsCover = curtains_ns.class_("CurtainsCover", cover.Cover, cg.Component)

CONFIG_SCHEMA = (
    cover.cover_schema(CurtainsCover)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = await cover.new_cover(config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
