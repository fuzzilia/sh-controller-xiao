#include <array>
#include "Arduino.h"
#include "ButtonMatrix.h"

// TODO モード選択ボタンにふさわしい番号を決める
#define SELECT_MODE_BUTTON_INDEX 0

static const std::array<uint8_t, 3> MatrixOutputPins = {PIN_A0, PIN_A1, PIN_A2};
static const std::array<uint8_t, 4> MatrixInputPins = {12, 11, 10, 9};

static int8_t lastOutputPinIndex = -1;

bool ButtonIsOn(int index)
{
  if (index < 3)
  {
    return digitalRead(MatrixInputPins[index]) == LOW;
  }
  else
  {
    return false;
  }
  uint8_t outputPinIndex = index / MatrixInputPins.size();
  uint8_t inputPinIndex = index % MatrixInputPins.size();
  if (outputPinIndex != lastOutputPinIndex)
  {
    analogWrite(MatrixOutputPins[outputPinIndex], 255);
    analogWrite(MatrixOutputPins[lastOutputPinIndex], 0);
    lastOutputPinIndex = outputPinIndex;
  }
  return digitalRead(MatrixOutputPins[inputPinIndex]) == LOW;
}

void InitPinsForButton()
{
  for (int i = 0; i < MatrixInputPins.size(); i++)
  {
    pinMode(MatrixInputPins[i], INPUT_PULLUP);
  }
  for (int i = 0; i < MatrixOutputPins.size(); i++)
  {
    pinMode(MatrixOutputPins[i], OUTPUT);
  }
}

bool ConfigButtonIsOn()
{
  return ButtonIsOn(SELECT_MODE_BUTTON_INDEX);
}