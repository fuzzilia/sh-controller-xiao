#include "LEDIndicator.h"

#include <Arduino.h>

LEDIndicator::LEDIndicator(int pin): m_pin(pin) {
    for (int i = 0; i < m_enabledEndressBlinkBrightPattern.size(); i++) {
        m_enabledEndressBlinkBrightPattern[i] = false;
    }
}

void LEDIndicator::beginEndressBlink(int periodTickCount) {
    m_endressBlinkPeriodTickCount = periodTickCount;
}

void LEDIndicator::enableEndressBlinkBrightPattern(EndressBlinkPattern patternNumber) {
    m_enabledEndressBlinkBrightPattern[(uint16_t)patternNumber] = true;
}

void LEDIndicator::disableEndressBlinkBrightPattern(EndressBlinkPattern patternNumber) {
    m_enabledEndressBlinkBrightPattern[(uint16_t)patternNumber] = false;
}

void LEDIndicator::initSingleBlink(int periodTickCount) {
    m_singleBlinkPeriodTickCount = periodTickCount;
}

void LEDIndicator::addSingleBlink(int count) {
    m_singleBlinkCount += count;
}

void LEDIndicator::tick(int tickCount) {
    if (m_singleBlinkPeriodTickCount > 0 && m_singleBlinkCount > 0) {
        // 回数限定の点滅が設定されていたらそちらを優先
        if (m_singleBlinkStartTickCount < 0) {
            m_singleBlinkStartTickCount = tickCount;
            return;
        }
        auto tickCountDiff = tickCount - m_singleBlinkStartTickCount;
        if (tickCountDiff < m_singleBlinkPeriodTickCount * 0.3) {
            // サイクル中の最初の30%の時間光る
            digitalWrite(m_pin, HIGH);
        } else {
            digitalWrite(m_pin, LOW);
        }

        // 1サイクル回ったときの処理
        if (tickCountDiff >= m_singleBlinkPeriodTickCount) {
            m_singleBlinkCount--;
            if (m_singleBlinkCount == 0) {
                m_singleBlinkStartTickCount = -1;
            } else {
                m_singleBlinkStartTickCount = m_singleBlinkStartTickCount + m_singleBlinkPeriodTickCount;
            }
        }
        return;
    }
    if (m_endressBlinkPeriodTickCount > 0) {
        auto tickCountInPeriod = tickCount % m_endressBlinkPeriodTickCount;
        
        if (m_enabledEndressBlinkBrightPattern[(uint16_t)EndressBlinkPattern::Wave]) {
            float blightness;
            if (tickCountInPeriod > m_endressBlinkPeriodTickCount / 2) {
                blightness = (m_endressBlinkPeriodTickCount - tickCountInPeriod) * ((255.0 * 2) / m_endressBlinkPeriodTickCount);
            } else {
                blightness = tickCountInPeriod * ((255.0 * 2) / m_endressBlinkPeriodTickCount);
            }
            analogWrite(m_pin, blightness);
            return;
        }

        bool isOn = false;
        if (m_enabledEndressBlinkBrightPattern[(uint16_t)EndressBlinkPattern::Full]) {
            isOn = true;
        } else {
            if (m_enabledEndressBlinkBrightPattern[(uint16_t)EndressBlinkPattern::Long]) {
                if (tickCountInPeriod < m_endressBlinkPeriodTickCount * 0.2) {
                    isOn = true;
                }
            }
            if (m_enabledEndressBlinkBrightPattern[(uint16_t)EndressBlinkPattern::Short]) {
                if (tickCountInPeriod >= m_endressBlinkPeriodTickCount * 0.5 && tickCountInPeriod < m_endressBlinkPeriodTickCount * 0.55) {
                    isOn = true;
                }
            }
        }
        digitalWrite(m_pin, isOn ? HIGH : LOW);
    }
}