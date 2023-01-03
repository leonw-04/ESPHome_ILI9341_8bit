#include "ili9341_8bit_display.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace ili9341_8bit {

static const char *const TAG = "ili9341_8bit";

void ILI9341_8bitDisplay::setup_pins_() {
  this->dc_pin_->setup();  // OUTPUT
  this->dc_pin_->digital_write(false);
  this->cs_pin_->setup();  // OUTPUT
  this->cs_pin_->digital_write(false);
  this->reset_pin_->setup();  // OUTPUT
  this->reset_pin_->digital_write(false);
  this->wr_pin_->setup();  // OUTPUT
  this->wr_pin_->digital_write(false);
  this->rd_pin_->setup();  // OUTPUT
  this->rd_pin_->digital_write(false);
  this->d0_pin_->setup();  // OUTPUT
  this->d0_pin_->digital_write(false);
  this->d1_pin_->setup();  // OUTPUT
  this->d1_pin_->digital_write(false);
  this->d2_pin_->setup();  // OUTPUT
  this->d2_pin_->digital_write(false);
  this->d3_pin_->setup();  // OUTPUT
  this->d3_pin_->digital_write(false);
  this->d4_pin_->setup();  // OUTPUT
  this->d4_pin_->digital_write(false);
  this->d5_pin_->setup();  // OUTPUT
  this->d5_pin_->digital_write(false);
  this->d6_pin_->setup();  // OUTPUT
  this->d6_pin_->digital_write(false);
  this->d7_pin_->setup();  // OUTPUT
  this->d7_pin_->digital_write(false);

  this->reset_();
}

void ILI9341_8bitDisplay::dump_config() {
  LOG_DISPLAY("", "ili9341_8bit", this);
  LOG_PIN("  CS Pin: ", this->cs_pin_);
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  LOG_PIN("  WR Pin: ", this->wr_pin_);
  LOG_PIN("  RD Pin: ", this->rd_pin_);
  LOG_PIN("  D0 Pin: ", this->d0_pin_);
  LOG_PIN("  D1 Pin: ", this->d1_pin_);
  LOG_PIN("  D2 Pin: ", this->d2_pin_);
  LOG_PIN("  D3 Pin: ", this->d3_pin_);
  LOG_PIN("  D4 Pin: ", this->d4_pin_);
  LOG_PIN("  D5 Pin: ", this->d5_pin_);
  LOG_PIN("  D6 Pin: ", this->d6_pin_);
  LOG_PIN("  D7 Pin: ", this->d7_pin_);
  LOG_UPDATE_INTERVAL(this);
}

float ILI9341_8bitDisplay::get_setup_priority() const { return setup_priority::HARDWARE; }

void ILI9341_8bitDisplay::command(uint8_t value) {
  this->start_command_();
  this->write_byte_(value);
  this->end_command_();
}

void ILI9341_8bitDisplay::reset_() {
  this->reset_pin_->digital_write(false);
  delay(10); 
  this->reset_pin_->digital_write(true);
  delay(10);
}

void ILI9341_8bitDisplay::data(uint8_t value) {
  this->start_data_();
  this->write_byte_(value);
  this->end_data_();
}

void ILI9341_8bitDisplay::send_command(uint8_t command_byte, const uint8_t *data_bytes, uint8_t num_data_bytes) {
  this->command(command_byte);  // Send the command byte
  this->start_data_();
  this->write_array_(data_bytes, num_data_bytes);
  this->end_data_();
}

uint8_t ILI9341_8bitDisplay::read_command(uint8_t command_byte, uint8_t index) {
  uint8_t data = 0x10 + index;
  this->send_command(0xD9, &data, 1);  // Set Index Register
  uint8_t result;
  this->start_command_();
  this->write_byte_(command_byte);
  this->start_read_();
  do {
    result = this->read_byte_();
  } while (index--);
  this->end_read_();
  return result;
}

void ILI9341_8bitDisplay::update() {
  this->do_update_();
  this->display_();
}

void ILI9341_8bitDisplay::display_() {
  // we will only update the changed window to the display
  uint16_t w = this->x_high_ - this->x_low_ + 1;
  uint16_t h = this->y_high_ - this->y_low_ + 1;
  uint32_t start_pos = ((this->y_low_ * this->width_) + x_low_);

  // check if something was displayed
  if ((this->x_high_ < this->x_low_) || (this->y_high_ < this->y_low_)) {
    return;
  }

  set_addr_window_(this->x_low_, this->y_low_, w, h);

  ESP_LOGVV("ILI9341_8bit", "Start ILI9341_8bitDisplay::display_(xl:%d, xh:%d, yl:%d, yh:%d, w:%d, h:%d, start_pos:%d)",
            this->x_low_, this->x_high_, this->y_low_, this->y_high_, w, h, start_pos);

  this->start_data_();
  for (uint16_t row = 0; row < h; row++) {
    uint32_t pos = start_pos + (row * width_);
    uint32_t rem = w;

    while (rem > 0) {
      uint32_t sz = buffer_to_transfer_(pos, rem);
      this->write_array_(transfer_buffer_, 2 * sz);
      pos += sz;
      rem -= sz;
      App.feed_wdt();
    }
    App.feed_wdt();
  }
  this->end_data_();

  // invalidate watermarks
  this->x_low_ = this->width_;
  this->y_low_ = this->height_;
  this->x_high_ = 0;
  this->y_high_ = 0;
}

void ILI9341_8bitDisplay::fill(Color color) {
  uint8_t color332 = 0;
  if (this->buffer_color_mode_ == BITS_8) {
    color332 = display::ColorUtil::color_to_332(color);
  } else {  // if (this->buffer_color_mode_ == BITS_8_INDEXED)
    color332 = display::ColorUtil::color_to_index8_palette888(color, this->palette_);
  }
  memset(this->buffer_, color332, this->get_buffer_length_());
  this->x_low_ = 0;
  this->y_low_ = 0;
  this->x_high_ = this->get_width_internal() - 1;
  this->y_high_ = this->get_height_internal() - 1;
}

void ILI9341_8bitDisplay::fill_internal_(uint8_t color) {
  memset(transfer_buffer_, color, sizeof(transfer_buffer_));

  uint32_t rem = (this->get_buffer_length_() * 2);

  this->set_addr_window_(0, 0, this->get_width_internal(), this->get_height_internal());
  this->start_data_();

  while (rem > 0) {
    size_t sz = rem <= sizeof(transfer_buffer_) ? rem : sizeof(transfer_buffer_);
    this->write_array_(transfer_buffer_, sz);
    rem -= sz;
  }

  this->end_data_();

  memset(buffer_, color, this->get_buffer_length_());
}

void ILI9341_8bitDisplay::rotate_my_(uint8_t m) {
  uint8_t rotation = m & 3;  // can't be higher than 3
  switch (rotation) {
    case 0:
      m = (MADCTL_MX | MADCTL_BGR);
      // _width = ILI9341_8bit_TFTWIDTH;
      // _height = ILI9341_8bit_TFTHEIGHT;
      break;
    case 1:
      m = (MADCTL_MV | MADCTL_BGR);
      // _width = ILI9341_8bit_TFTHEIGHT;
      // _height = ILI9341_8bit_TFTWIDTH;
      break;
    case 2:
      m = (MADCTL_MY | MADCTL_BGR);
      // _width = ILI9341_8bit_TFTWIDTH;
      // _height = ILI9341_8bit_TFTHEIGHT;
      break;
    case 3:
      m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
      // _width = ILI9341_8bit_TFTHEIGHT;
      // _height = ILI9341_8bit_TFTWIDTH;
      break;
  }

  this->command(ILI9341_8bit_MADCTL);
  this->data(m);
}

void HOT ILI9341_8bitDisplay::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x >= this->get_width_internal() || x < 0 || y >= this->get_height_internal() || y < 0)
    return;

  uint32_t pos = (y * width_) + x;
  uint8_t new_color;

  if (this->buffer_color_mode_ == BITS_8) {
    new_color = display::ColorUtil::color_to_332(color, display::ColorOrder::COLOR_ORDER_RGB);
  } else {  // if (this->buffer_color_mode_ == BITS_8_INDEXED) {
    new_color = display::ColorUtil::color_to_index8_palette888(color, this->palette_);
  }

  if (buffer_[pos] != new_color) {
    buffer_[pos] = new_color;
    // low and high watermark may speed up drawing from buffer
    this->x_low_ = (x < this->x_low_) ? x : this->x_low_;
    this->y_low_ = (y < this->y_low_) ? y : this->y_low_;
    this->x_high_ = (x > this->x_high_) ? x : this->x_high_;
    this->y_high_ = (y > this->y_high_) ? y : this->y_high_;
  }
}

// should return the total size: return this->get_width_internal() * this->get_height_internal() * 2 // 16bit color
// values per bit is huge
uint32_t ILI9341_8bitDisplay::get_buffer_length_() { return this->get_width_internal() * this->get_height_internal(); }

void ILI9341_8bitDisplay::start_command_() {
  this->wr_pin_->digital_write(false);
}

void ILI9341_8bitDisplay::end_command_() { 
  this->cs_pin_->digital_write(false);
  this->rd_pin_->digital_write(true);
  this->dc_pin_->digital_write(false);
  this->wr_pin_->digital_write(true);
}
void ILI9341_8bitDisplay::start_data_() {
  this->wr_pin_->digital_write(false);
}
void ILI9341_8bitDisplay::end_data_() {
  this->cs_pin_->digital_write(false);
  this->rd_pin_->digital_write(true);
  this->dc_pin_->digital_write(true);
  this->wr_pin_->digital_write(true);
}
void ILI9341_8bitDisplay::start_read_() {
  this->cs_pin_->digital_write(false);
  this->dc_pin_->digital_write(true);
  this->wr_pin_->digital_write(true);
}
void ILI9341_8bitDisplay::end_read_(){}

void ILI9341_8bitDisplay::write_byte_(uint8_t value) {
  this->d0_pin_->digital_write(value & (1 << 0));
  this->d1_pin_->digital_write(value & (1 << 1));
  this->d2_pin_->digital_write(value & (1 << 2));
  this->d3_pin_->digital_write(value & (1 << 3));
  this->d4_pin_->digital_write(value & (1 << 4));
  this->d5_pin_->digital_write(value & (1 << 5));
  this->d6_pin_->digital_write(value & (1 << 6));
  this->d7_pin_->digital_write(value & (1 << 7));
}

uint8_t ILI9341_8bitDisplay::read_byte_() {
  while (!this->rd_pin_->digital_read());
  bool d0_state = this->d0_pin_->digital_read();
  bool d1_state = this->d1_pin_->digital_read();
  bool d2_state = this->d2_pin_->digital_read();
  bool d3_state = this->d3_pin_->digital_read();
  bool d4_state = this->d4_pin_->digital_read();
  bool d5_state = this->d5_pin_->digital_read();
  bool d6_state = this->d6_pin_->digital_read();
  bool d7_state = this->d7_pin_->digital_read();
  return (uint8_t)((d0_state << 0) | (d1_state << 1) | (d2_state << 2) | (d3_state << 3) | (d4_state << 4) | (d5_state << 5) | (d6_state << 6) | (d7_state << 7));
}

void ILI9341_8bitDisplay::write_array_(const uint8_t *data_bytes, uint8_t num_data_bytes) {
  for (int i=0; i<num_data_bytes; i++) {
    this->wr_pin_->digital_write(false);
    this->write_byte_(*(data_bytes+i));
    this->wr_pin_->digital_write(true);
  }
}


void ILI9341_8bitDisplay::init_lcd_(const uint8_t *init_cmd) {
  uint8_t cmd, x, num_args;
  const uint8_t *addr = init_cmd;
  while ((cmd = progmem_read_byte(addr++)) > 0) {
    x = progmem_read_byte(addr++);
    num_args = x & 0x7F;
    send_command(cmd, addr, num_args);
    addr += num_args;
    if (x & 0x80)
      delay(150);  // NOLINT
  }
}

void ILI9341_8bitDisplay::set_addr_window_(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h) {
  uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);
  this->command(ILI9341_8bit_CASET);  // Column address set
  this->start_data_();
  this->write_byte_(x1 >> 8);
    this->wr_pin_->digital_write(false);
  this->write_byte_(x1);
    this->wr_pin_->digital_write(true);
    this->wr_pin_->digital_write(false);
  this->write_byte_(x2 >> 8);
    this->wr_pin_->digital_write(true);
    this->wr_pin_->digital_write(false);
  this->write_byte_(x2);
  this->end_data_();
  this->command(ILI9341_8bit_PASET);  // Row address set
  this->start_data_();
  this->write_byte_(y1 >> 8);
    this->wr_pin_->digital_write(true);
    this->wr_pin_->digital_write(false);
  this->write_byte_(y1);
    this->wr_pin_->digital_write(true);
    this->wr_pin_->digital_write(false);
  this->write_byte_(y2 >> 8);
    this->wr_pin_->digital_write(true);
    this->wr_pin_->digital_write(false);
  this->write_byte_(y2);
  this->end_data_();
  this->command(ILI9341_8bit_RAMWR);  // Write to RAM
}

void ILI9341_8bitDisplay::invert_display_(bool invert) { this->command(invert ? ILI9341_8bit_INVON : ILI9341_8bit_INVOFF); }

int ILI9341_8bitDisplay::get_width_internal() { return this->width_; }
int ILI9341_8bitDisplay::get_height_internal() { return this->height_; }

uint32_t ILI9341_8bitDisplay::buffer_to_transfer_(uint32_t pos, uint32_t sz) {
  uint8_t *src = buffer_ + pos;
  uint8_t *dst = transfer_buffer_;

  if (sz > sizeof(transfer_buffer_) / 2) {
    sz = sizeof(transfer_buffer_) / 2;
  }

  for (uint32_t i = 0; i < sz; ++i) {
    uint16_t color;
    if (this->buffer_color_mode_ == BITS_8) {
      color = display::ColorUtil::color_to_565(display::ColorUtil::rgb332_to_color(*src++));
    } else {  //  if (this->buffer_color_mode == BITS_8_INDEXED) {
      Color col = display::ColorUtil::index8_to_color_palette888(*src++, this->palette_);
      color = display::ColorUtil::color_to_565(col);
    }
    *dst++ = (uint8_t)(color >> 8);
    *dst++ = (uint8_t) color;
  }

  return sz;
}

//   M5Stack display
void ILI9341_8bitM5Stack::initialize() {
  this->init_lcd_(INITCMD_M5STACK);
  this->width_ = 320;
  this->height_ = 240;
  this->invert_display_(true);
}

//   24_TFT display
void ILI9341_8bitTFT24::initialize() {
  this->init_lcd_(INITCMD_TFT);
  this->width_ = 240;
  this->height_ = 320;
}

//   24_TFT rotated display
void ILI9341_8bitTFT24R::initialize() {
  this->init_lcd_(INITCMD_TFT);
  this->width_ = 320;
  this->height_ = 240;
}

}  // namespace ili9341_8bit
}  // namespace esphome