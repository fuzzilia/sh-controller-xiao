#include <Adafruit_MPR121.h>
#include <Adafruit_DotStar.h>
#include <bluefruit.h>
#include <InternalFileSystem.h>
#include <array>
#include "FreeRTOS.h"
#include "src/lib/SHController.h"
#include "CL3GD20.h"
#include "ConfigLoader.h"
#include "KeyService.h"
#include "KeyConfigService.h"
#include "ButtonMatrix.h"
#include "AnalogStick.h"
#include "nrf_rng.h"
#include "nrf_soc.h"
#include "Common.h"

#define FRAME_PER_SEC 60
#define READ_MOTION_PER_FRAME 3
static float READ_MOTION_PERIOD_MS = 1.0f / FRAME_PER_SEC / READ_MOTION_PER_FRAME;

Adafruit_DotStar strip(1, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLOCK, DOTSTAR_BRG);

// static CL3GD20 *l2gd20 = nullptr; // ジャイロセンサー.

BLEDis bledis;

static int readMotionCount = 0;

static SHController *sh_controller = nullptr;
static bool isConfigMode = false;
static portTickType startTick;

std::vector<MotionSensorValue> MotionSensorValues()
{
  return {
      MotionSensorValue{.time_span_s = 1.0f / 60 / 3, .gyro = {.x = 0.0f, .y = 0.0f, .z = 0.0f}},
      MotionSensorValue{.time_span_s = 1.0f / 60 / 3, .gyro = {.x = 0.0f, .y = 0.0f, .z = 0.0f}},
      MotionSensorValue{.time_span_s = 1.0f / 60 / 3, .gyro = {.x = 0.0f, .y = 0.0f, .z = 0.0f}},
  };
}

static void GenerateRandomAddress(ble_gap_addr_t &addr)
{
  addr.addr_id_peer = 0;
  addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
  for (int i = 0; i < BLE_GAP_ADDR_LEN; i++) {
    addr.addr[i] = 0;
  }

  uint8_t availableBytes;
  do {
    DEBUG_PRINT("generating...");
    delay(10);
    sd_rand_application_bytes_available_get(&availableBytes);
  } while (availableBytes < BLE_GAP_ADDR_LEN);
  sd_rand_application_vector_get(addr.addr, BLE_GAP_ADDR_LEN);
  addr.addr[5] |= 0xC0;

#ifndef SH_CONTROLLER_PRODUCTION
  Serial.println("generated.");
  for (int i = 0; i < BLE_GAP_ADDR_LEN; i++) {
    Serial.print(":");
    Serial.println(addr.addr[i], HEX);
  }
  Serial.println("");
  Serial.flush();
#endif
}

static void showMode(bool isConfigMode)
{
  if (isConfigMode)
  {
    DEBUG_PRINT("config mode");
    strip.setPixelColor(0, 0, 0, 20);
  }
  else
  {
    DEBUG_PRINT("key mode");
    strip.setPixelColor(0, 0, 20, 0);
  }
  strip.show();
}

static void startAdv(void)
{
  if (!isConfigMode)
  {
    initKeyService();

    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
    Bluefruit.Advertising.addName();
  }
  else
  {
    ble_gap_addr_t addr;
    GenerateRandomAddress(addr);
    Bluefruit.setAddr(&addr);

    Bluefruit.setName("SH-CONF");

    initKeyConfigService();

    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addName();
  }

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
  bool isOk = Bluefruit.Advertising.start(0);
  if (isOk) {
    DEBUG_PRINT("start adv.");
  } else {
    DEBUG_PRINT("adv failed.");
  }
}

static void OnConnect(uint16_t connection_handle) {
  DEBUG_PRINT("on connect");
}

void setup()
{
  strip.begin();
  strip.setBrightness(2);
  strip.show();
  Serial.begin(115200);
  while (!Serial)
    delay(10); // for nrf52840 with native usb

  analogReadResolution(12);
  InitPinsForButton();
  InitPinsForStick();

  InternalFS.begin();
  strip.setPixelColor(0, 255, 255, 0);
  strip.show();
  isConfigMode = ConfigButtonIsOn();

  showMode(isConfigMode);

  DEBUG_PRINT("start load");
  auto config = ConfigLoader::load();
#ifndef SH_CONTROLLER_PRODUCTION
  {
    auto configString = config->ToString();
    Serial.println(configString.c_str());
    Serial.flush();
  }
#endif
  sh_controller = new SHController(std::move(config), ButtonIsOn, StickValue, MotionSensorValues);

  Bluefruit.begin();
  Bluefruit.setTxPower(4); // Check bluefruit.h for supported values
  Bluefruit.setName("SH-CON2");

#ifndef SH_CONTROLLER_PRODUCTION
  Bluefruit.Periph.setConnectCallback(OnConnect);
#endif

  // // Configure and Start Device Information Service
  bledis.setManufacturer("FUZZILIA");
  bledis.setModel("SH-CONTROLLER-NRF52");
  bledis.begin();

  startAdv();
  if (sh_controller->config().NeedsSensorInput())
  {
    // l2gd20 = new CL3GD20();
  }

  DEBUG_PRINT("start");
  startTick = xTaskGetTickCount();
}

void loop()
{
  if (isConfigMode) {
    DEBUG_PRINT("config...");
    vTaskDelay(10000);
  } else {
    vTaskDelay(configTICK_RATE_HZ / 64 / 2);
    
    readMotionCount++;
    if (readMotionCount >= 2)
    {
      RefreshStickValue();
      auto keys = sh_controller->tick();
      for (auto key : keys) {
        sendKey(key);
      }
      readMotionCount = 0;
    }
  }
}
