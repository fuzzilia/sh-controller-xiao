#include <Arduino.h>
#include <bluefruit.h>
#include "KeyConfigService.h"
#include "KeyConfigLoader.h"

const uint8_t keyConfigServiceUuid[] =
    {0x74, 0x6a, 0x3e, 0xec, 0x17, 0x98, 0xc2, 0x9d, 0x74, 0x4d, 0xb5, 0x31, 0x2d, 0x6d, 0x21, 0x34};

const uint8_t keyConfigCharacteristicUuid[] =
    {0x9b, 0x96, 0x05, 0x34, 0x3b, 0xc5, 0x8a, 0x9c, 0xfc, 0x4b, 0x16, 0x3c, 0x42, 0x1e, 0xb4, 0x61};

BLEService keyConfigService(keyConfigServiceUuid);
BLECharacteristic keyConfigCharacteristic(keyConfigCharacteristicUuid);

void onWriteKeyConfig(uint16_t handle, BLECharacteristic *characteristic, uint8_t *data, uint16_t size)
{
    if (characteristic == &keyConfigCharacteristic)
    {
        Serial.println("onWrite");
        Serial.print("Size : ");
        Serial.print(size);
        Serial.println("");
        KeyConfigLoader::save(data, size);
    }
    else
    {
        Serial.println("uuid is not match @ onWrite");
    }
}

void initKeyConfigService(const KeyConfig &keyConfig, boolean isReadonly)
{
    keyConfigService.begin();

    keyConfigCharacteristic.setProperties(CHR_PROPS_WRITE);
    keyConfigCharacteristic.setPermission(SECMODE_OPEN, SECMODE_OPEN);
    keyConfigCharacteristic.begin();
    keyConfigCharacteristic.setWriteCallback(onWriteKeyConfig);

    Bluefruit.Advertising.addService(keyConfigService);
}
