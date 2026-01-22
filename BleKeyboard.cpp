#include "BleKeyboard.h"

#include "sdkconfig.h"
#include <driver/adc.h>
#include "esp_timer.h"
#include "esp_log.h"

static const char* LOG_TAG = "BleKeyboard";

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

  inputKeyboard  = hid->inputReport(KEYBOARD_ID);
  outputKeyboard = hid->outputReport(KEYBOARD_ID);
  inputMediaKeys = hid->inputReport(MEDIA_KEYS_ID);

  outputKeyboard->setCallbacks(this);

  hid->manufacturer()->setValue(deviceManufacturer);

  hid->pnp(0x02, vid, pid, version);
  hid->hidInfo(0x00, 0x01);

  hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  hid->startServices();

  advertising = NimBLEDevice::getAdvertising();
  advertising->setAppearance(HID_KEYBOARD);
  advertising->addServiceUUID(hid->hidService()->getUUID());
  advertising->setScanResponse(false);
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

void BleKeyboard::onConnect(NimBLEServer* pServer) {
  (void)pServer;
  connected = true;
}

void BleKeyboard::onDisconnect(NimBLEServer* pServer) {
  (void)pServer;
  connected = false;
  if (advertising) advertising->start();
}

void BleKeyboard::onWrite(NimBLECharacteristic* me) {
  (void)me;
}

void BleKeyboard::delay_ms(uint64_t ms) {
  if (!ms) return;
  uint64_t start = esp_timer_get_time();
  uint64_t end = start + (ms * 1000ULL);
  while (esp_timer_get_time() < end) {}
}
