import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core, pins
from esphome.components import display, spi
from esphome.const import (
    CONF_COLOR_PALETTE,
    CONF_DC_PIN,
    CONF_ID,
    CONF_LAMBDA,
    CONF_MODEL,
    CONF_PAGES,
    CONF_RAW_DATA_ID,
    CONF_RESET_PIN,
)
from esphome.core import CORE, HexInt

DEPENDENCIES = ["spi"]

CONF_COLOR_PALETTE_IMAGES = "color_palette_images"
CONF_LED_PIN = "led_pin"

ili9341_8bit_ns = cg.esphome_ns.namespace("ili9341_8bit")
ili9341_8bit = ili9341_8bit_ns.class_(
    "ILI9341_8bitDisplay", cg.PollingComponent, spi.SPIDevice, display.DisplayBuffer
)
ILI9341_8bitM5Stack = ili9341_8bit_ns.class_("ILI9341_8bitM5Stack", ili9341_8bit)
ILI9341_8bitTFT24 = ili9341_8bit_ns.class_("ILI9341_8bitTFT24", ili9341_8bit)
ILI9341_8bitTFT24R = ili9341_8bit_ns.class_("ILI9341_8bitTFT24R", ili9341_8bit)

ILI9341_8bitModel = ili9341_8bit_ns.enum("ILI9341_8bitModel")
ILI9341_8bitColorMode = ili9341_8bit_ns.enum("ILI9341_8bitColorMode")

MODELS = {
    "M5STACK": ILI9341_8bitModel.M5STACK,
    "TFT_2.4": ILI9341_8bitModel.TFT_24,
    "TFT_2.4R": ILI9341_8bitModel.TFT_24R,
}

ILI9341_8bit_MODEL = cv.enum(MODELS, upper=True, space="_")

COLOR_PALETTE = cv.one_of("NONE", "GRAYSCALE", "IMAGE_ADAPTIVE")


def _validate(config):
    if config.get(CONF_COLOR_PALETTE) == "IMAGE_ADAPTIVE" and not config.get(
        CONF_COLOR_PALETTE_IMAGES
    ):
        raise cv.Invalid(
            "Color palette in IMAGE_ADAPTIVE mode requires at least one 'color_palette_images' entry to generate palette"
        )
    if (
        config.get(CONF_COLOR_PALETTE_IMAGES)
        and config.get(CONF_COLOR_PALETTE) != "IMAGE_ADAPTIVE"
    ):
        raise cv.Invalid(
            "Providing color palette images requires palette mode to be 'IMAGE_ADAPTIVE'"
        )
    return config


CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(ili9341_8bit),
            cv.Required(CONF_MODEL): ILI9341_8bit_MODEL,
            cv.Required(CONF_CS_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_DC_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_WR_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_RD_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_D0_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_D1_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_D2_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_D3_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_D4_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_D5_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_D6_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_D7_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_COLOR_PALETTE, default="NONE"): COLOR_PALETTE,
            cv.Optional(CONF_COLOR_PALETTE_IMAGES, default=[]): cv.ensure_list(
                cv.file_
            ),
            cv.GenerateID(CONF_RAW_DATA_ID): cv.declare_id(cg.uint8),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(spi.spi_device_schema(False)),
    cv.has_at_most_one_key(CONF_PAGES, CONF_LAMBDA),
    _validate,
)


async def to_code(config):
    if config[CONF_MODEL] == "M5STACK":
        lcd_type = ILI9341_8bitM5Stack
    if config[CONF_MODEL] == "TFT_2.4":
        lcd_type = ILI9341_8bitTFT24
    if config[CONF_MODEL] == "TFT_2.4R":
        lcd_type = ILI9341_8bitTFT24R
    rhs = lcd_type.new()
    var = cg.Pvariable(config[CONF_ID], rhs)

    await cg.register_component(var, config)
    await display.register_display(var, config)
    await spi.register_spi_device(var, config)
    cg.add(var.set_model(config[CONF_MODEL]))
    cs = await cg.gpio_pin_expression(config[CONF_CS_PIN])
    cg.add(var.set_dc_pin(cs))
    dc = await cg.gpio_pin_expression(config[CONF_DC_PIN])
    cg.add(var.set_dc_pin(dc))
    reset = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
    cg.add(var.set_dc_pin(reset))
    wr = await cg.gpio_pin_expression(config[CONF_WR_PIN])
    cg.add(var.set_dc_pin(wr))
    rd = await cg.gpio_pin_expression(config[CONF_RD_PIN])
    cg.add(var.set_dc_pin(rd))
    d0 = await cg.gpio_pin_expression(config[CONF_D0_PIN])
    cg.add(var.set_dc_pin(d0))
    d1 = await cg.gpio_pin_expression(config[CONF_D1_PIN])
    cg.add(var.set_dc_pin(d1))
    d2 = await cg.gpio_pin_expression(config[CONF_D2_PIN])
    cg.add(var.set_dc_pin(d2))
    d3 = await cg.gpio_pin_expression(config[CONF_D3_PIN])
    cg.add(var.set_dc_pin(d3))
    d4 = await cg.gpio_pin_expression(config[CONF_D4_PIN])
    cg.add(var.set_dc_pin(d4))
    d5 = await cg.gpio_pin_expression(config[CONF_D5_PIN])
    cg.add(var.set_dc_pin(d5))
    d6 = await cg.gpio_pin_expression(config[CONF_D6_PIN])
    cg.add(var.set_dc_pin(d6))
    d7 = await cg.gpio_pin_expression(config[CONF_D7_PIN])
    cg.add(var.set_dc_pin(d7))

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayBufferRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))

    rhs = None
    if config[CONF_COLOR_PALETTE] == "GRAYSCALE":
        cg.add(var.set_buffer_color_mode(ILI9341_8bitColorMode.BITS_8_INDEXED))
        rhs = []
        for x in range(256):
            rhs.extend([HexInt(x), HexInt(x), HexInt(x)])
    elif config[CONF_COLOR_PALETTE] == "IMAGE_ADAPTIVE":
        cg.add(var.set_buffer_color_mode(ILI9341_8bitColorMode.BITS_8_INDEXED))
        from PIL import Image

        def load_image(filename):
            path = CORE.relative_config_path(filename)
            try:
                return Image.open(path)
            except Exception as e:
                raise core.EsphomeError(f"Could not load image file {path}: {e}")

        # make a wide horizontal combined image.
        images = [load_image(x) for x in config[CONF_COLOR_PALETTE_IMAGES]]
        total_width = sum(i.width for i in images)
        max_height = max(i.height for i in images)

        ref_image = Image.new("RGB", (total_width, max_height))
        x = 0
        for i in images:
            ref_image.paste(i, (x, 0))
            x = x + i.width

        # reduce the colors on combined image to 256.
        converted = ref_image.convert("P", palette=Image.ADAPTIVE, colors=256)
        # if you want to verify how the images look use
        # ref_image.save("ref_in.png")
        # converted.save("ref_out.png")
        palette = converted.getpalette()
        assert len(palette) == 256 * 3
        rhs = palette
    else:
        cg.add(var.set_buffer_color_mode(ILI9341_8bitColorMode.BITS_8))

    if rhs is not None:
        prog_arr = cg.progmem_array(config[CONF_RAW_DATA_ID], rhs)
        cg.add(var.set_palette(prog_arr))