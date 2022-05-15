
   
#include <Arduino.h>
#include <bluefruit.h>
#include "KeyConfigService.h"
#include "ConfigLoader.h"

const uint8_t keyConfigServiceUuid[] =
    {0x5F, 0x52, 0xF7, 0x79, 0x8B, 0x72, 0xDD, 0xA8, 0x23, 0x45, 0x54, 0x6B, 0x1C, 0xDC, 0xFD, 0x20};

const uint8_t keyConfigCharacteristicUuid[] =
    {0xEE, 0x47, 0x6A, 0x54, 0x53, 0xB3, 0x79, 0x8E, 0x8C, 0x4B, 0x85, 0x74, 0xAE, 0xF2, 0x96, 0xAE};

BLEService keyConfigService(keyConfigServiceUuid);
BLECharacteristic keyConfigCharacteristic(keyConfigCharacteristicUuid);

static void onWriteKeyConfig(uint16_t handle, BLECharacteristic *characteristic, uint8_t *data, uint16_t size)
{
    if (characteristic == &keyConfigCharacteristic)
    {
        Serial.println("onWrite");
        Serial.print("Size : ");
        Serial.print(size);
        Serial.println("");

        {
            SHConfig config(data);
            if (!config.isValid()) {
                // 本当はBLEレベルでの書き込み失敗扱いにしたいが、ライブラリの作りの問題で不可能っぽい
                return;
            }
        }

        ConfigLoader::save(data, size);
    }
    else
    {
        Serial.println("uuid is not match @ onWrite");
    }
}

void initKeyConfigService()
{
    keyConfigService.begin();

    keyConfigCharacteristic.setProperties(CHR_PROPS_WRITE | CHR_PROPS_READ);
    keyConfigCharacteristic.setPermission(SECMODE_OPEN, SECMODE_OPEN);
    keyConfigCharacteristic.begin();
    keyConfigCharacteristic.setWriteCallback(onWriteKeyConfig);

    Bluefruit.Advertising.addService(keyConfigService);
}