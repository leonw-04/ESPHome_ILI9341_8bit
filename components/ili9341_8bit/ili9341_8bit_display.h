#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "ili9341_8bit_defines.h"
#include "ili9341_8bit_init.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ili9341_8bit {

enum ILI9341_8bitModel {
  M5STACK = 0,
  TFT_24,
  TFT_24R,
};

enum ILI9341_8bitColorMode {
  BITS_8,
  BITS_8_INDEXED,
};

class ILI9341_8bitDisplay : public PollingComponent,
                       public display::DisplayBuffer {
 public:
  void set_cs_pin(GPIOPin *cs_pin) { cs_pin_ = cs_pin; }
  void set_dc_pin(GPIOPin *dc_pin) { dc_pin_ = dc_pin; }
  void set_reset_pin(GPIOPin *reset) { reset_pin_ = reset_pin; }
  void set_wr_pin(GPIOPin *wr_pin) { wr_pin_ = wr_pin; }
  void set_rd_pin(GPIOPin *rd_pin) { rd_pin_ = rd_pin; }
  void set_d0_pin(GPIOPin *d0_pin) { d0_pin_ = d0_pin; }
  void set_d1_pin(GPIOPin *d1_pin) { d1_pin_ = d1_pin; }
  void set_d2_pin(GPIOPin *d2_pin) { d2_pin_ = d2_pin; }
  void set_d3_pin(GPIOPin *d3_pin) { d3_pin_ = d3_pin; }
  void set_d4_pin(GPIOPin *d4_pin) { d4_pin_ = d4_pin; }
  void set_d5_pin(GPIOPin *d5_pin) { d5_pin_ = d5_pin; }
  void set_d6_pin(GPIOPin *d6_pin) { d6_pin_ = d6_pin; }
  void set_d7_pin(GPIOPin *d7_pin) { d7_pin_ = d7_pin; }
  float get_setup_priority() const override;
  void set_model(ILI9341_8bitModel model) { this->model_ = model; }
  void set_palette(const uint8_t *palette) { this->palette_ = palette; }
  void set_buffer_color_mode(ILI9341_8bitColorMode color_mode) { this->buffer_color_mode_ = color_mode; }

  void command(uint8_t value);
  void data(uint8_t value);
  void send_command(uint8_t command_byte, const uint8_t *data_bytes, uint8_t num_data_bytes);
  uint8_t read_command(uint8_t command_byte, uint8_t index);
  virtual void initialize() = 0;

  void update() override;

  void fill(Color color) override;

  void dump_config() override;
  void setup() override {
    this->setup_pins_();
    this->initialize();

    this->x_low_ = this->width_;
    this->y_low_ = this->height_;
    this->x_high_ = 0;
    this->y_high_ = 0;

    this->init_internal_(this->get_buffer_length_());
    this->fill_internal_(0x00);
  }

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  void setup_pins_();

  void init_lcd_(const uint8_t *init_cmd);
  void set_addr_window_(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
  void invert_display_(bool invert);
  void reset_();
  void fill_internal_(uint8_t color);
  void display_();
  void rotate_my_(uint8_t m);

  ILI9341_8bitModel model_;
  int16_t width_{320};   ///< Display width as modified by current rotation
  int16_t height_{240};  ///< Display height as modified by current rotation
  uint16_t x_low_{0};
  uint16_t y_low_{0};
  uint16_t x_high_{0};
  uint16_t y_high_{0};
  const uint8_t *palette_;

  ILI9341_8bitColorMode buffer_color_mode_{BITS_8};

  uint32_t get_buffer_length_();
  int get_width_internal() override;
  int get_height_internal() override;

  void start_command_();
  void end_command_();
  void start_data_();
  void end_data_();
  void start_read_();
  void end_read_();
  void write_byte_(uint8_t value);

  uintt8 read_byte_(uint8_t value);

  void write_array_(const uint8_t *data_bytes, uint8_t num_data_bytes);

  uint8_t transfer_buffer_[64];

  uint32_t buffer_to_transfer_(uint32_t pos, uint32_t sz);

  GPIOPin *reset_pin_{nullptr};
  GPIOPin *led_pin_{nullptr};
  GPIOPin *dc_pin_;
  GPIOPin *busy_pin_{nullptr};
};

//-----------   M5Stack display --------------
class ILI9341_8bitM5Stack : public ILI9341_8bitDisplay {
 public:
  void initialize() override;
};

//-----------   ILI9341_8bit_24_TFT display --------------
class ILI9341_8bitTFT24 : public ILI9341_8bitDisplay {
 public:
  void initialize() override;
};

//-----------   ILI9341_8bit_24_TFT rotated display --------------
class ILI9341_8bitTFT24R : public ILI9341_8bitDisplay {
 public:
  void initialize() override;
};

}  // namespace ili9341_8bit
}  // namespace esphome