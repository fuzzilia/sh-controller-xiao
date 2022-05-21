#include "AnalogStick.h"

#include "Arduino.h"

#define X_INPUT_PIN PIN_A3
#define Y_INPUT_PIN PIN_A4

#define SENSOR_VALUE_MARGIN 64
// 12bit 解像度
#define VALUE_RESOLUTION 0x1000
#define CENTER_VALUE (VALUE_RESOLUTION / 2)
#define SENSOR_MAX_UINT_VALUE (VALUE_RESOLUTION - SENSOR_VALUE_MARGIN)
#define SENSOR_MIN_UINT_VALUE (SENSOR_VALUE_MARGIN)

static float xValue = 0.0f;
static float yValue = 0.0f;

float StickValue(int, TwoDimension dimension) {
    switch (dimension) {
        case TwoDimension::X:
            return xValue;
        case TwoDimension::Y:
            return yValue;
        default:
            return 0.0f;
    }
}

static float AnalogValueToFloat(uint32_t uintValue) {
    if (uintValue >= SENSOR_MAX_UINT_VALUE) {
        return 1.0f;
    }
    if (uintValue <= SENSOR_MIN_UINT_VALUE) {
        return -1.0f;
    }
    int32_t intValue = uintValue - CENTER_VALUE;
    return ((float)intValue / ((float)VALUE_RESOLUTION / 2 - SENSOR_VALUE_MARGIN));
}

void RefreshStickValue() {
    // アナログ値読み取りはやや時間のかかる処理のため、
    // 念の為値の読み出しはStickValue関数とは分けておく。
    xValue = AnalogValueToFloat(analogRead(X_INPUT_PIN));
    yValue = AnalogValueToFloat(analogRead(Y_INPUT_PIN));
}

void InitPinsForStick() {
    // ピンのデフォルト設定でOKなので、初期化必要なし
}