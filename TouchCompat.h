#pragma once

#include <Arduino.h>

// Unified touch read helper to avoid repeating board-specific glue.
// Returns true when a valid touch is detected and fills x/y in screen coordinates.
//
// Requirements:
//   - When USECAPTOUCH is defined: a global `ts` object exists with `touched()` and `getPoint()`.
//     * FT6236 library returns TS_Point {x,y} roughly in panel space but may need rotation fixes.
//     * Our Waveshare 4.3B GT911 wrapper returns screen-space coordinates (0..SCREEN_WIDTH/HEIGHT).
//   - Otherwise: TFT_eSPI resistive touch path with `tft.getTouch(&x,&y)`.
inline bool read_touch(uint16_t &t_x, uint16_t &t_y) {
#ifdef USECAPTOUCH
  if (ts.touched()) {
    auto p = ts.getPoint();

    // Default: treat p.x/p.y as screen coordinates.
    // Legacy FT6236 path in this project expected a 320-wide coordinate space and swapped axes.
    // Keep that behavior unless overridden by board define.
#if defined(WAVESHARE_ESP32S3_TOUCH_LCD_43B)
    t_x = (uint16_t)p.x;
    t_y = (uint16_t)p.y;
#else
    // Map FT6236 coordinates into the current SCREEN_* space.
    // Old code:
    //   p.x = map(p.x, 0, 320, 320, 0);
    //   t_y = p.x;
    //   t_x = p.y;
    // We preserve the same axis swap, but scale using SCREEN_WIDTH/HEIGHT.
    int32_t px = p.x;
    int32_t py = p.y;
    px = map(px, 0, 320, 320, 0);
    t_y = (uint16_t)map(px, 0, 320, 0, SCREEN_HEIGHT);
    t_x = (uint16_t)map(py, 0, 480, 0, SCREEN_WIDTH);
#endif
    return true;
  }
  return false;
#else
  return tft.getTouch(&t_x, &t_y);
#endif
}
