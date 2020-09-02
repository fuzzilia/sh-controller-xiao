#include <Arduino.h>
#include <bluefruit.h>
#include "KeyConfigService.h"
#include "KeyConfigLoader.h"

const uint8_t keyConfigServiceUuid[] = 
    { 0x34, 0x21, 0x6d, 0x2d, 0x31, 0xb5, 0x4d, 0x74, 0x9d, 0xc2, 0x98, 0x17, 0xec, 0x3e, 0x6a, 0x74 };

const uint8_t keyConfigCharacteristicUuid[] = 
    { 0x61, 0xb4, 0x1e, 0x42, 0x3c, 0x16, 0x4b, 0xfc, 0x9c, 0x8a, 0xc5, 0x3b, 0x34, 0x05, 0x96, 0x9b };

BLEService keyConfigService(keyConfigServiceUuid);
BLECharacteristic keyConfigCharacteristic(keyConfigCharacteristicUuid);

void onWriteKeyConfig(uint16_t handle, BLECharacteristic *characteristic, uint8_t* data, uint16_t size) {
    if (keyConfigService.uuid == handle) {
        Serial.println("onWrite");
        Serial.print("Size : ");
        Serial.print(size);
        Serial.println("");
        KeyConfigLoader::save(data, size);
    } else {
        Serial.println("uuid is not match @ onWrite");
    }
}

void initKeyConfigService(const KeyConfig &keyConfig, boolean isReadonly) {
    keyConfigService.begin();

    keyConfigCharacteristic.setProperties(CHR_PROPS_WRITE);
    keyConfigCharacteristic.setPermission(SECMODE_OPEN, SECMODE_OPEN);
    keyConfigCharacteristic.begin();
    keyConfigCharacteristic.setWriteCallback(onWriteKeyConfig);

    Bluefruit.Advertising.addService(keyConfigService);
}
