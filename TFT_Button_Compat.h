#pragma once

#include <Arduino.h>

// Minimal drop-in replacement for TFT_eSPI_Button used by this project.
// Used when building for boards that do not provide TFT_eSPI's button helper.
//
// Supported methods:
//   - initButton(&tft, x, y, w, h, outline, fill, textcolor, label, textsize)
//   - drawButton(inverted=false)
//   - contains(x,y)
//   - press(bool p)
//   - justPressed() / justReleased()

template <typename GFX>
class TFT_Button_Compat {
public:
  void initButton(GFX *gfx, int16_t x, int16_t y, uint16_t w, uint16_t h,
                  uint16_t outline, uint16_t fill, uint16_t textcolor,
                  const char *label, uint8_t textsize) {
    _gfx = gfx;
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _outline = outline;
    _fill = fill;
    _text = textcolor;
    _label = label ? label : "";
    _textsize = textsize;
  }

  void drawButton(bool inverted = false) {
    if (!_gfx) return;
    uint16_t fill = inverted ? _text : _fill;
    uint16_t text = inverted ? _fill : _text;

    // Draw a simple filled rounded rect + centered label.
    _gfx->fillRoundRect(_x - _w / 2, _y - _h / 2, _w, _h, 6, fill);
    _gfx->drawRoundRect(_x - _w / 2, _y - _h / 2, _w, _h, 6, _outline);

    _gfx->setTextColor(text, fill);
    _gfx->setTextSize(_textsize);
    // LovyanGFX supports setTextDatum/drawString; TFT_eSPI does too. Use if available.
    #ifdef MC_DATUM
      _gfx->setTextDatum(MC_DATUM);
      _gfx->drawString(_label, _x, _y);
    #else
      _gfx->setCursor(_x - (int16_t)(strlen(_label) * 6 * _textsize) / 2, _y - 4 * _textsize);
      _gfx->print(_label);
    #endif
  }

  bool contains(int16_t x, int16_t y) const {
    return (x >= (_x - _w / 2)) && (x <= (_x + _w / 2)) &&
           (y >= (_y - _h / 2)) && (y <= (_y + _h / 2));
  }

  void press(bool p) {
    _last = _curr;
    _curr = p;
  }

  bool justPressed() const { return (_curr && !_last); }
  bool justReleased() const { return (!_curr && _last); }

private:
  GFX *_gfx = nullptr;
  int16_t _x = 0, _y = 0;
  uint16_t _w = 0, _h = 0;
  uint16_t _outline = 0, _fill = 0, _text = 0;
  const char *_label = "";
  uint8_t _textsize = 1;
  bool _curr = false;
  bool _last = false;
};
