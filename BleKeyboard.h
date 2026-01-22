#pragma once
#include <Arduino.h>

#if !defined(USE_NIMBLE)
  #error "This BleKeyboard implementation requires USE_NIMBLE=1"
#endif

#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>

#include "HIDTypes.h"

// Report IDs:
#define KEYBOARD_ID    0x01
#define MEDIA_KEYS_ID  0x02

class BleKeyboard : public Print,
                    public NimBLEServerCallbacks,
                    public NimBLECharacteristicCallbacks {
public:
  BleKeyboard(std::string deviceName = "FreeTouchDeck",
              std::string deviceManufacturer = "FreeTouchDeck",
              uint8_t batteryLevel = 100);

  void begin(void);
  void end(void);

  bool isConnected(void);

  void setBatteryLevel(uint8_t level);
  void setName(std::string deviceName);
  void setDelay(uint32_t ms);

  void set_vendor_id(uint16_t vid);
  void set_product_id(uint16_t pid);
  void set_version(uint16_t version);

  // Print
  size_t write(uint8_t c) override;
  size_t write(const uint8_t *buffer, size_t size) override;

  // Keyboard
  size_t press(uint8_t k);
  size_t release(uint8_t k);
  void releaseAll(void);

  // Media keys
  size_t press(const MediaKeyReport k);
  size_t release(const MediaKeyReport k);
  size_t write(const MediaKeyReport c);

  // Callbacks
  void onConnect(NimBLEServer* pServer) override;
  void onDisconnect(NimBLEServer* pServer) override;
  void onWrite(NimBLECharacteristic* me) override;

private:
  void sendReport(KeyReport* keys);
  void sendReport(MediaKeyReport* keys);
  void delay_ms(uint64_t ms);

  NimBLEHIDDevice*       hid = nullptr;
  NimBLECharacteristic*  inputKeyboard = nullptr;
  NimBLECharacteristic*  outputKeyboard = nullptr;
  NimBLECharacteristic*  inputMediaKeys = nullptr;

  NimBLEAdvertising*     advertising = nullptr;

  bool connected = false;

  std::string deviceName;
  std::string deviceManufacturer;

  uint8_t batteryLevel = 100;
  uint32_t _delay_ms = 8;

  uint16_t vid = 0x05AC;     // default-ish
  uint16_t pid = 0x0220;
  uint16_t version = 0x0110;

  KeyReport _keyReport{};
  MediaKeyReport _mediaKeyReport{0,0};
};
