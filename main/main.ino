
#include <bluefruit.h>
#include <InternalFileSystem.h>
#include <array>
#include "FreeRTOS.h"
#include "src/lib/SHController.h"
#include "ConfigLoader.h"
#include "KeyService.h"
#include "KeyConfigService.h"
#include "ButtonMatrix.h"
#include "AnalogStick.h"
#include "LEDIndicator.h"
#include "nrf_rng.h"
#include "nrf_soc.h"
#include "Common.h"
#include <queue>

#define FRAME_PER_SEC 60
#define READ_MOTION_PER_FRAME 3
#define LED_PIN 13
#define GYRO_SENSOR_ADDRESS 0x6A
static float READ_MOTION_PERIOD_MS = 1.0f / FRAME_PER_SEC / READ_MOTION_PER_FRAME;

BLEDis bledis;
LEDIndicator led(LED_PIN);

static int readMotionCount = 0;

static SHController *sh_controller = nullptr;
static bool isConfigMode = false;
static portTickType startTick;
static bool isGyroEnabled = false;

static TaskHandle_t sendKeyTaskHandle;
static std::queue<KeyboardValue> sendingKeys;

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
    delay(1);
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
    // strip.setPixelColor(0, 0, 0, 120);
  }
  else
  {
    DEBUG_PRINT("key mode");
    // strip.setPixelColor(0, 0, 120, 0);
  }
  // strip.show();
}

static void startAdv(void)
{
  if (!isConfigMode)
  {
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);

    initKeyService();

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

static void OnConnect(uint16_t connectionHandle) {
  DEBUG_PRINT("on connect");
  led.disableEndressBlinkBrightPattern(LEDIndicator::EndressBlinkPattern::Short);
  led.enableEndressBlinkBrightPattern(LEDIndicator::EndressBlinkPattern::Full);
  led.addSingleBlink(3);
}

static void OnDisconnect(uint16_t connectionHandle, uint8_t reason) {
  DEBUG_PRINT("on disconnect");
  DEBUG_PRINT(reason);
  led.enableEndressBlinkBrightPattern(LEDIndicator::EndressBlinkPattern::Short);
  led.disableEndressBlinkBrightPattern(LEDIndicator::EndressBlinkPattern::Full);
}

static void SendKeyTask(void *arg)
{
  (void) arg;

  while (true) {
    while (!sendingKeys.empty()) {
      sendKey(sendingKeys.front());
      sendingKeys.pop();
    }
    vTaskDelay(configTICK_RATE_HZ / 64);
  }
}

void setup()
{
#ifndef SH_CONTROLLER_PRODUCTION
  Serial.begin(115200);
  while (!Serial)
    delay(10); // for nrf52840 with native usb
#endif

  analogReadResolution(12);
  InitPinsForButton();
  InitPinsForStick();
  pinMode(LED_PIN, OUTPUT);

  InternalFS.begin();
  isConfigMode = ConfigButtonIsOn();

  showMode(isConfigMode);
  if (isConfigMode) {
    led.beginEndressBlink(configTICK_RATE_HZ * 3);
    led.enableEndressBlinkBrightPattern(LEDIndicator::EndressBlinkPattern::Wave);
  } else {
    led.beginEndressBlink(configTICK_RATE_HZ * 1.5);
    led.initSingleBlink(configTICK_RATE_HZ * 0.8);
    led.enableEndressBlinkBrightPattern(LEDIndicator::EndressBlinkPattern::Short);
  }

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

  Bluefruit.Periph.setConnectCallback(OnConnect);
  Bluefruit.Periph.setDisconnectCallback(OnDisconnect);

  // // Configure and Start Device Information Service
  bledis.setManufacturer("FUZZILIA");
  bledis.setModel("SH-CONTROLLER-NRF52");
  bledis.begin();

  startAdv();
  // if (!isConfigMode)
  // // if (!isConfigMode && sh_controller->config().NeedsSensorInput())
  // {
  //   DEBUG_PRINT("initialize gyro.");

  //   // アドレス設定がAdafruit_L3GD20_Unifiedの前提と異なっているので、Adafruit_L3GD20_Unifiedは使わずAdafruit_L3GD20を使う方針
  //   // だが、Adafruit_L3GD20内で利用している_i2cをAdafruit_L3GD20経由で初期化する方法が無いので、
  //   // Adafruit_L3GD20_Unifiedのbeginメソッドを利用して初期化する。
  //   Adafruit_L3GD20_Unified gyroForInit;
  //   gyroForInit.begin();
  //   auto isGyroEnabled = gyro.begin(GYRO_RANGE_250DPS, GYRO_SENSOR_ADDRESS);
  //   if (!isGyroEnabled) {
  //     DEBUG_PRINT("fail to initialize gyro.");
  //   }
  // }

  xTaskCreate(SendKeyTask, "SendKeyTask", 1024, nullptr, TASK_PRIO_LOW, &sendKeyTaskHandle);

  DEBUG_PRINT("start");
  startTick = xTaskGetTickCount();
}

static int loopCount = 0;

void loop()
{
  loopCount++;
  auto localStartTickCount = xTaskGetTickCount();
  led.tick(localStartTickCount);
  if (isConfigMode) {
    DEBUG_PRINT("config...");
    vTaskDelay(configTICK_RATE_HZ / 32);
  } else {
    
    readMotionCount++;
    if (readMotionCount >= 2)
    {
      readMotionCount = 0;

      RefreshStickValue();
      sh_controller->tick(sendingKeys);

#ifndef SH_CONTROLLER_PRODUCTION
      if (loopCount % (64 * 2) == 0) {
        for (int i = 0; i < 12; i++) {
          if (ButtonIsOn(i)) {
            Serial.print("1");
          } else {
            Serial.print("0");
          }
        }

        Serial.print(" Stick X=");
        Serial.print(StickValue(0, TwoDimension::X), 3);
        Serial.print(" Y=");
        Serial.print(StickValue(0, TwoDimension::Y), 3);
        Serial.println("");

        // gyro.read();
        // auto readEndTick = xTaskGetTickCount();
        // Serial.print("X: "); Serial.print((int)gyro.data.x);   Serial.print(" ");
        // Serial.print("Y: "); Serial.print((int)gyro.data.y);   Serial.print(" ");
        // Serial.print("Z: "); Serial.println((int)gyro.data.z); Serial.print(" ");
        // Serial.println("");
        // Serial.flush();
      }
#endif
    }
    auto tickCountDiff = xTaskGetTickCount() - localStartTickCount;
    auto delayTickCount = configTICK_RATE_HZ / 64 / 2 - tickCountDiff;
    if (delayTickCount > 0) {
      vTaskDelay(delayTickCount);
    }
  }
}
