#include <array>
#include "Arduino.h"
#include "ButtonMatrix.h"

static const std::array<uint8_t, 3> MatrixOutputPins = {PIN_A0, PIN_A1, PIN_A2};
static const std::array<uint8_t, 4> MatrixInputPins = {12, 11, 10, 9};

static uint8_t lastOutputPinIndex = 0;

bool ButtonIsOn(int index) {
    uint8_t outputPinIndex = index / MatrixInputPins.size();
    uint8_t inputPinIndex = index % MatrixInputPins.size();
    if (outputPinIndex != lastOutputPinIndex) {
        analogWrite(MatrixOutputPins[outputPinIndex], 255);
        analogWrite(MatrixOutputPins[lastOutputPinIndex], 0);
        lastOutputPinIndex = outputPinIndex;
    }
    return digitalRead(inputPinIndex) == LOW;
}