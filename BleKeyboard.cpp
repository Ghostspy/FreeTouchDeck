#include "BleKeyboard.h"

#include "sdkconfig.h"
#include <driver/adc.h>
#include "esp_timer.h"
#include "esp_log.h"

static const char* LOG_TAG = "BleKeyboard";

// ASCII to HID keycode mapping
const uint8_t _asciimap[128] PROGMEM = {
  0x00,             // NUL
  0x00,             // SOH
  0x00,             // STX
  0x00,             // ETX
  0x00,             // EOT
  0x00,             // ENQ
  0x00,             // ACK
  0x00,             // BEL
  0x2a,             // BS  Backspace
  0x2b,             // TAB Tab
  0x28,             // LF  Enter
  0x00,             // VT
  0x00,             // FF
  0x00,             // CR
  0x00,             // SO
  0x00,             // SI
  0x00,             // DEL
  0x00,             // DC1
  0x00,             // DC2
  0x00,             // DC3
  0x00,             // DC4
  0x00,             // NAK
  0x00,             // SYN
  0x00,             // ETB
  0x00,             // CAN
  0x00,             // EM
  0x00,             // SUB
  0x00,             // ESC
  0x00,             // FS
  0x00,             // GS
  0x00,             // RS
  0x00,             // US
  0x2c,             //  ' '
  0x1e|0x80,        // !
  0x34|0x80,        // "
  0x20|0x80,        // #
  0x21|0x80,        // $
  0x22|0x80,        // %
  0x24|0x80,        // &
  0x34,             // '
  0x26|0x80,        // (
  0x27|0x80,        // )
  0x25|0x80,        // *
  0x2e|0x80,        // +
  0x36,             // ,
  0x2d,             // -
  0x37,             // .
  0x38,             // /
  0x27,             // 0
  0x1e,             // 1
  0x1f,             // 2
  0x20,             // 3
  0x21,             // 4
  0x22,             // 5
  0x23,             // 6
  0x24,             // 7
  0x25,             // 8
  0x26,             // 9
  0x33|0x80,        // :
  0x33,             // ;
  0x36|0x80,        // <
  0x2e,             // =
  0x37|0x80,        // >
  0x38|0x80,        // ?
  0x1f|0x80,        // @
  0x04|0x80,        // A
  0x05|0x80,        // B
  0x06|0x80,        // C
  0x07|0x80,        // D
  0x08|0x80,        // E
  0x09|0x80,        // F
  0x0a|0x80,        // G
  0x0b|0x80,        // H
  0x0c|0x80,        // I
  0x0d|0x80,        // J
  0x0e|0x80,        // K
  0x0f|0x80,        // L
  0x10|0x80,        // M
  0x11|0x80,        // N
  0x12|0x80,        // O
  0x13|0x80,        // P
  0x14|0x80,        // Q
  0x15|0x80,        // R
  0x16|0x80,        // S
  0x17|0x80,        // T
  0x18|0x80,        // U
  0x19|0x80,        // V
  0x1a|0x80,        // W
  0x1b|0x80,        // X
  0x1c|0x80,        // Y
  0x1d|0x80,        // Z
  0x2f,             // [
  0x31,             // bslash
  0x30,             // ]
  0x23|0x80,        // ^
  0x2d|0x80,        // _
  0x35,             // `
  0x04,             // a
  0x05,             // b
  0x06,             // c
  0x07,             // d
  0x08,             // e
  0x09,             // f
  0x0a,             // g
  0x0b,             // h
  0x0c,             // i
  0x0d,             // j
  0x0e,             // k
  0x0f,             // l
  0x10,             // m
  0x11,             // n
  0x12,             // o
  0x13,             // p
  0x14,             // q
  0x15,             // r
  0x16,             // s
  0x17,             // t
  0x18,             // u
  0x19,             // v
  0x1a,             // w
  0x1b,             // x
  0x1c,             // y
  0x1d,             // z
  0x2f|0x80,        // {
  0x31|0x80,        // |
  0x30|0x80,        // }
  0x35|0x80,        // ~
  0x00              // DEL
};

// HID descriptor (keyboard + consumer/media)
static const uint8_t _hidReportDescriptor[] = {
  USAGE_PAGE(1),      0x01,
  USAGE(1),           0x06,
  COLLECTION(1),      0x01,

  REPORT_ID(1),       KEYBOARD_ID,
  USAGE_PAGE(1),      0x07,
  USAGE_MINIMUM(1),   0xE0,
  USAGE_MAXIMUM(1),   0xE7,
  LOGICAL_MINIMUM(1), 0x00,
  LOGICAL_MAXIMUM(1), 0x01,
  REPORT_SIZE(1),     0x01,
  REPORT_COUNT(1),    0x08,
  HIDINPUT(1),        0x02,

  REPORT_COUNT(1),    0x01,
  REPORT_SIZE(1),     0x08,
  HIDINPUT(1),        0x01,

  REPORT_COUNT(1),    0x05,
  REPORT_SIZE(1),     0x01,
  USAGE_PAGE(1),      0x08,
  USAGE_MINIMUM(1),   0x01,
  USAGE_MAXIMUM(1),   0x05,
  HIDOUTPUT(1),       0x02,

  REPORT_COUNT(1),    0x01,
  REPORT_SIZE(1),     0x03,
  HIDOUTPUT(1),       0x01,

  REPORT_COUNT(1),    0x06,
  REPORT_SIZE(1),     0x08,
  LOGICAL_MINIMUM(1), 0x00,
  LOGICAL_MAXIMUM(1), 0x65,
  USAGE_PAGE(1),      0x07,
  USAGE_MINIMUM(1),   0x00,
  USAGE_MAXIMUM(1),   0x65,
  HIDINPUT(1),        0x00,
  END_COLLECTION(0),

  USAGE_PAGE(1),      0x0C,
  USAGE(1),           0x01,
  COLLECTION(1),      0x01,

  REPORT_ID(1),       MEDIA_KEYS_ID,
  USAGE_PAGE(1),      0x0C,
  LOGICAL_MINIMUM(1), 0x00,
  LOGICAL_MAXIMUM(1), 0x01,
  REPORT_SIZE(1),     0x01,
  REPORT_COUNT(1),    0x10,

  USAGE(1),           0xB5,
  USAGE(1),           0xB6,
  USAGE(1),           0xB7,
  USAGE(1),           0xCD,
  USAGE(1),           0xE2,
  USAGE(1),           0xE9,
  USAGE(1),           0xEA,
  USAGE(2),           0x23, 0x02,
  USAGE(2),           0x94, 0x01,
  USAGE(2),           0x92, 0x01,
  USAGE(2),           0x2A, 0x02,
  USAGE(2),           0x21, 0x02,
  USAGE(2),           0x26, 0x02,
  USAGE(2),           0x24, 0x02,
  USAGE(2),           0x83, 0x01,
  USAGE(2),           0x8A, 0x01,

  HIDINPUT(1),        0x02,
  END_COLLECTION(0)
};

BleKeyboard::BleKeyboard(std::string deviceName_, std::string deviceManufacturer_, uint8_t batteryLevel_)
  : deviceName(deviceName_.substr(0, 15))
  , deviceManufacturer(deviceManufacturer_.substr(0, 15))
  , batteryLevel(batteryLevel_) {}

void BleKeyboard::begin(void) {
  NimBLEDevice::init(deviceName);

  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(this);

  hid = new NimBLEHIDDevice(pServer);

  inputKeyboard  = hid->getInputReport(KEYBOARD_ID);
  outputKeyboard = hid->getOutputReport(KEYBOARD_ID);
  inputMediaKeys = hid->getInputReport(MEDIA_KEYS_ID);

  outputKeyboard->setCallbacks(this);

  hid->setManufacturer(deviceManufacturer);

  hid->setPnp(0x02, vid, pid, version);
  hid->setHidInfo(0x00, 0x01);

  hid->setReportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  hid->startServices();

  advertising = NimBLEDevice::getAdvertising();
  advertising->setAppearance(HID_KEYBOARD);
  advertising->addServiceUUID(hid->getHidService()->getUUID());
  advertising->start();

  hid->setBatteryLevel(batteryLevel);

  ESP_LOGI(LOG_TAG, "NimBLE HID advertising started");
}

void BleKeyboard::end(void) {
  // optional: NimBLEDevice::deinit(true);
}

bool BleKeyboard::isConnected(void) {
  return connected;
}

void BleKeyboard::setBatteryLevel(uint8_t level) {
  batteryLevel = level;
  if (hid) hid->setBatteryLevel(batteryLevel);
}

void BleKeyboard::setName(std::string name) {
  deviceName = name.substr(0, 15);
}

void BleKeyboard::setDelay(uint32_t ms) { _delay_ms = ms; }
void BleKeyboard::set_vendor_id(uint16_t v) { vid = v; }
void BleKeyboard::set_product_id(uint16_t p) { pid = p; }
void BleKeyboard::set_version(uint16_t v) { version = v; }

void BleKeyboard::sendReport(KeyReport* keys) {
  if (!connected) return;
  inputKeyboard->setValue((uint8_t*)keys, sizeof(KeyReport));
  inputKeyboard->notify();
  delay_ms(_delay_ms);
}

void BleKeyboard::sendReport(MediaKeyReport* keys) {
  if (!connected) return;
  inputMediaKeys->setValue((uint8_t*)keys, sizeof(MediaKeyReport));
  inputMediaKeys->notify();
  delay_ms(_delay_ms);
}

size_t BleKeyboard::press(uint8_t k) {
  uint8_t i;
  if (k >= 136) {
    k = k - 136;
  } else if (k >= 128) {
    _keyReport.modifiers |= (1 << (k - 128));
    k = 0;
  } else {
    extern const uint8_t _asciimap[128] PROGMEM;
    k = pgm_read_byte(_asciimap + k);
    if (!k) { setWriteError(); return 0; }
    if (k & 0x80) { _keyReport.modifiers |= 0x02; k &= 0x7F; }
  }

  if (_keyReport.keys[0] != k && _keyReport.keys[1] != k &&
      _keyReport.keys[2] != k && _keyReport.keys[3] != k &&
      _keyReport.keys[4] != k && _keyReport.keys[5] != k) {
    for (i = 0; i < 6; i++) {
      if (_keyReport.keys[i] == 0x00) { _keyReport.keys[i] = k; break; }
    }
    if (i == 6) { setWriteError(); return 0; }
  }

  sendReport(&_keyReport);
  return 1;
}

size_t BleKeyboard::release(uint8_t k) {
  uint8_t i;

  if (k >= 136) {
    k = k - 136;
  } else if (k >= 128) {
    _keyReport.modifiers &= ~(1 << (k - 128));
    k = 0;
  } else {
    extern const uint8_t _asciimap[128] PROGMEM;
    k = pgm_read_byte(_asciimap + k);
    if (!k) return 0;
    if (k & 0x80) { _keyReport.modifiers &= ~(0x02); k &= 0x7F; }
  }

  for (i = 0; i < 6; i++) {
    if (k != 0 && _keyReport.keys[i] == k) _keyReport.keys[i] = 0x00;
  }

  sendReport(&_keyReport);
  return 1;
}

void BleKeyboard::releaseAll(void) {
  memset(_keyReport.keys, 0, sizeof(_keyReport.keys));
  _keyReport.modifiers = 0;
  _mediaKeyReport[0] = 0;
  _mediaKeyReport[1] = 0;
  sendReport(&_keyReport);
  sendReport(&_mediaKeyReport);
}

size_t BleKeyboard::press(const MediaKeyReport k) {
  uint16_t k_16 = k[1] | (k[0] << 8);
  uint16_t r_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8);
  r_16 |= k_16;
  _mediaKeyReport[0] = (uint8_t)((r_16 & 0xFF00) >> 8);
  _mediaKeyReport[1] = (uint8_t)(r_16 & 0x00FF);
  sendReport(&_mediaKeyReport);
  return 1;
}

size_t BleKeyboard::release(const MediaKeyReport k) {
  uint16_t k_16 = k[1] | (k[0] << 8);
  uint16_t r_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8);
  r_16 &= ~k_16;
  _mediaKeyReport[0] = (uint8_t)((r_16 & 0xFF00) >> 8);
  _mediaKeyReport[1] = (uint8_t)(r_16 & 0x00FF);
  sendReport(&_mediaKeyReport);
  return 1;
}

size_t BleKeyboard::write(const MediaKeyReport c) {
  size_t p = press(c);
  release(c);
  return p;
}

size_t BleKeyboard::write(uint8_t c) {
  uint8_t p = press(c);
  release(c);
  return p;
}

size_t BleKeyboard::write(const uint8_t *buffer, size_t size) {
  size_t n = 0;
  while (size--) {
    if (*buffer != '\r') {
      if (write(*buffer)) n++;
      else break;
    }
    buffer++;
  }
  return n;
}

void BleKeyboard::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
  (void)pServer;
  (void)connInfo;
  connected = true;
}

void BleKeyboard::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
  (void)pServer;
  (void)connInfo;
  (void)reason;
  connected = false;
  if (advertising) advertising->start();
}

void BleKeyboard::onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
  (void)pCharacteristic;
  (void)connInfo;
}

void BleKeyboard::delay_ms(uint64_t ms) {
  if (!ms) return;
  uint64_t start = esp_timer_get_time();
  uint64_t end = start + (ms * 1000ULL);
  while (esp_timer_get_time() < end) {}
}
