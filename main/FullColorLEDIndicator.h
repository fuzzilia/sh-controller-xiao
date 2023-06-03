#ifndef SH_CONTROLLER_FULL_COLOR_LED_INDICATOR_H
#define SH_CONTROLLER_FULL_COLOR_LED_INDICATOR_H

#include <array>

class FullColorLEDIndicator {
public:
    enum class Mode {
        // 利用モード : 待機中
        UsageWaiting,

        // 利用モード1
        Usage1,

        // 利用モード2
        Usage2,

        // 利用モード3
        Usage3,

        // 設定モード
        Config,
    };

private:
    std::array<uint8_t, 3> m_pins;
    Mode m_mode;

public:
    FullColorLEDIndicator(const std::array<uint8_t, 3> &pins);
    void setMode(Mode mode);
}

#endif // SH_CONTROLLER_FULL_COLOR_LED_INDICATOR_H
