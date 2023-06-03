#include "FullColorLEDIndicator.h"

FullColorLEDIndicator::FullColorLEDIndicator(const std::array<uint8_t, 3> &pins): m_pins(pins) {}

void FullColorLEDIndicator::setMode(Mode mode) {
    switch (mode)
    {
    case Mode::UsageWaiting:
        // 切断されたとき用の処理
        break;
    
    default:
        break;
    }
    m_mode = mode;
}