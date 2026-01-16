// Waveshare ESP32-S3-Touch-LCD-4.3B (RGB 800x480 + GT911 touch + CH422G IO expander)
//
// This header provides:
//   - A TFT_eSPI-like display object (LovyanGFX) exposed as `tft`
//   - Capacitive touch (GT911 over I2C) exposed as `ts` with `touched()` / `getPoint()`
//   - Backlight + touch reset control via CH422G (I2C IO expander)
//
// Pin mapping is taken from the Waveshare wiki (LCD RGB + Touch I2C + CH422G EXIO pins).
// You can adjust RGB timing values below if your panel shows tearing/rolling/blank.

#pragma once

#include <Arduino.h>
#include <Wire.h>

// --- I2C (Touch + CH422G) ---
// SDA = GPIO8, SCL = GPIO9
#ifndef WS43B_I2C_SDA
#  define WS43B_I2C_SDA 8
#endif
#ifndef WS43B_I2C_SCL
#  define WS43B_I2C_SCL 9
#endif

// Touch interrupt pin (GPIO4)
#ifndef WS43B_TOUCH_INT
#  define WS43B_TOUCH_INT 4
#endif

// CH422G expanded pins (EXIO1 = TP_RST, EXIO2 = DISP backlight enable)
// Note: CH422G is controlled over I2C. Address is library-defined; override if needed.
#ifndef WS43B_CH422G_ADDR
// Common default for CH422G modules is 0x24, but board revisions may differ.
#  define WS43B_CH422G_ADDR 0x24
#endif

// --- Display (RGB) ---
// Panel resolution
static constexpr int WS43B_LCD_W = 800;
static constexpr int WS43B_LCD_H = 480;

// RGB timing (tune if needed)
// These defaults are typical for 800x480 RGB panels; adjust if your panel is unstable.
#ifndef WS43B_PCLK_HZ
#  define WS43B_PCLK_HZ 16000000
#endif
#ifndef WS43B_HSYNC_PULSE
#  define WS43B_HSYNC_PULSE 1
#endif
#ifndef WS43B_HBP
#  define WS43B_HBP 46
#endif
#ifndef WS43B_HFP
#  define WS43B_HFP 210
#endif
#ifndef WS43B_VSYNC_PULSE
#  define WS43B_VSYNC_PULSE 1
#endif
#ifndef WS43B_VBP
#  define WS43B_VBP 23
#endif
#ifndef WS43B_VFP
#  define WS43B_VFP 22
#endif

// --- Libraries ---
// Display: LovyanGFX (RGB panel + TFT_eSPI-like API)
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>

// Provide common TFT_eSPI color constants if they are not already defined.
#ifndef TFT_BLACK
#  define TFT_BLACK 0x0000
#endif
#ifndef TFT_WHITE
#  define TFT_WHITE 0xFFFF
#endif
#ifndef TFT_RED
#  define TFT_RED   0xF800
#endif
#ifndef TFT_GREEN
#  define TFT_GREEN 0x07E0
#endif
#ifndef TFT_BLUE
#  define TFT_BLUE  0x001F
#endif
#ifndef TFT_YELLOW
#  define TFT_YELLOW 0xFFE0
#endif
#ifndef TFT_CYAN
#  define TFT_CYAN  0x07FF
#endif
#ifndef TFT_MAGENTA
#  define TFT_MAGENTA 0xF81F
#endif

// IO expander: ESP32_IO_Expander (CH422G)
// Library: https://github.com/esp-arduino-libs/ESP32_IO_Expander
#include <ESP_IOExpander.h>
#include "chip/esp_expander_ch422g.hpp"

// --- Simple GT911 touch driver (single point) ---
// This is intentionally small: we only need one coordinate for keypad taps.
class GT911_Simple {
public:
  struct Point { int16_t x; int16_t y; };

  bool begin(TwoWire &wire, uint8_t intPin, esp_expander::CH422G &io,
            uint8_t rstExioPin, uint8_t addr1 = 0x5D, uint8_t addr2 = 0x14) {
    _wire = &wire;
    _intPin = intPin;
    _io = &io;
    _rstExio = rstExioPin;
    _addr1 = addr1;
    _addr2 = addr2;
    pinMode(_intPin, INPUT);

    // Reset sequence via CH422G EXIO pin
    if (_io) {
      _io->pinMode(_rstExio, OUTPUT);
      _io->digitalWrite(_rstExio, LOW);
      delay(10);
      _io->digitalWrite(_rstExio, HIGH);
      delay(50);
    }

    // Probe address
    _addr = probe(_addr1) ? _addr1 : (probe(_addr2) ? _addr2 : 0);
    return _addr != 0;
  }

  bool touched() {
    if (_addr == 0) return false;
    // Status register 0x814E: bit7 = data ready, low bits = number of points
    uint8_t status = 0;
    if (!readReg(0x814E, &status, 1)) return false;
    return (status & 0x80) && ((status & 0x0F) > 0);
  }

  Point getPoint() {
    Point p{0, 0};
    if (_addr == 0) return p;
    uint8_t buf[8] = {0};
    // First point data starts at 0x8150
    if (!readReg(0x8150, buf, sizeof(buf))) return p;
    // GT911: X = buf[1]<<8 | buf[0], Y = buf[3]<<8 | buf[2]
    p.x = (int16_t)((buf[1] << 8) | buf[0]);
    p.y = (int16_t)((buf[3] << 8) | buf[2]);

    // Clear data-ready flag by writing 0 to 0x814E
    uint8_t zero = 0;
    writeReg(0x814E, &zero, 1);
    return p;
  }

private:
  TwoWire *_wire = nullptr;
  esp_expander::CH422G *_io = nullptr;
  uint8_t _intPin = 0;
  uint8_t _rstExio = 0;
  uint8_t _addr = 0;
  uint8_t _addr1 = 0x5D;
  uint8_t _addr2 = 0x14;

  bool probe(uint8_t addr) {
    _wire->beginTransmission(addr);
    return _wire->endTransmission() == 0;
  }

  bool writeReg(uint16_t reg, const uint8_t *data, size_t len) {
    if (!_wire) return false;
    _wire->beginTransmission(_addr);
    _wire->write((uint8_t)(reg & 0xFF));
    _wire->write((uint8_t)(reg >> 8));
    _wire->write(data, len);
    return _wire->endTransmission() == 0;
  }

  bool readReg(uint16_t reg, uint8_t *out, size_t len) {
    if (!_wire) return false;
    _wire->beginTransmission(_addr);
    _wire->write((uint8_t)(reg & 0xFF));
    _wire->write((uint8_t)(reg >> 8));
    if (_wire->endTransmission(false) != 0) return false;
    size_t n = _wire->requestFrom((int)_addr, (int)len);
    if (n != len) return false;
    for (size_t i = 0; i < len; i++) out[i] = _wire->read();
    return true;
  }
};

// --- LovyanGFX panel config for ESP32-S3 RGB ---
class LGFX_WS43B : public lgfx::LGFX_Device {
public:
  LGFX_WS43B() {
    {
      auto cfg = _bus.config();
      // RGB bus pins (from Waveshare wiki)
      cfg.panel = &_panel;
      cfg.pin_d0  = 39; // G2
      cfg.pin_d1  = 0;  // G3
      cfg.pin_d2  = 45; // G4
      cfg.pin_d3  = 48; // G5
      cfg.pin_d4  = 47; // G6
      cfg.pin_d5  = 21; // G7

      cfg.pin_d6  = 14; // B3
      cfg.pin_d7  = 38; // B4
      cfg.pin_d8  = 18; // B5
      cfg.pin_d9  = 17; // B6
      cfg.pin_d10 = 10; // B7

      cfg.pin_d11 = 1;  // R3
      cfg.pin_d12 = 2;  // R4
      cfg.pin_d13 = 42; // R5
      cfg.pin_d14 = 41; // R6
      cfg.pin_d15 = 40; // R7

      cfg.pin_henable = 5;  // DE
      cfg.pin_vsync   = 3;  // VSYNC
      cfg.pin_hsync   = 46; // HSYNC
      cfg.pin_pclk    = 7;  // PCLK

      cfg.freq_write  = WS43B_PCLK_HZ;
      cfg.hsync_polarity = 0;
      cfg.hsync_front_porch = WS43B_HFP;
      cfg.hsync_pulse_width = WS43B_HSYNC_PULSE;
      cfg.hsync_back_porch  = WS43B_HBP;

      cfg.vsync_polarity = 0;
      cfg.vsync_front_porch = WS43B_VFP;
      cfg.vsync_pulse_width = WS43B_VSYNC_PULSE;
      cfg.vsync_back_porch  = WS43B_VBP;

      cfg.pclk_active_neg = 1;
      cfg.de_idle_high    = 0;
      cfg.pclk_idle_high  = 0;

      _bus.config(cfg);
    }
    {
      auto pcfg = _panel.config();
      pcfg.memory_width  = WS43B_LCD_W;
      pcfg.memory_height = WS43B_LCD_H;
      pcfg.panel_width   = WS43B_LCD_W;
      pcfg.panel_height  = WS43B_LCD_H;
      pcfg.offset_x      = 0;
      pcfg.offset_y      = 0;
      // pcfg.rotation      = 0;
      pcfg.readable      = false;
      pcfg.invert        = false;
      pcfg.rgb_order     = false;
      pcfg.dlen_16bit    = true;
      pcfg.bus_shared    = false;
      _panel.config(pcfg);
    }
    _panel.setBus(&_bus);
    setPanel(&_panel);
  }

private:
  lgfx::v1::Bus_RGB _bus;
  lgfx::v1::Panel_RGB _panel;
};

// Global objects expected by the rest of the project
extern LGFX_WS43B tft;
extern esp_expander::CH422G ioexp;
extern GT911_Simple ts;

// Initialize board peripherals (call once in setup)
inline bool waveshare43b_begin() {
  Wire.begin(WS43B_I2C_SDA, WS43B_I2C_SCL);

  ioexp.begin();   // no args

  ioexp.pinMode(2, OUTPUT);
  ioexp.digitalWrite(2, HIGH);

  bool ok = ts.begin(Wire, WS43B_TOUCH_INT, ioexp, 1);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  return ok;
}
