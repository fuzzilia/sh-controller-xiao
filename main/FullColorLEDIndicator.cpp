#include "FullColorLEDIndicator.h"
#include <Arduino.h>
#include "Common.h"

static const Color DEFAULT_COLOR = Color{0, 0, 0};
static const Color CONFIG_COLOR = Color{0xFF, 0x00, 0xFF};
static const Color STANDARD_COLOR = Color{0xFF, 0xFF, 0xFF};
static const Color MODE1_COLOR = Color{0x00, 0xFF, 0x00};

static std::unique_ptr<FullColorLEDPattern> MakeStandardAdvertizingPattern()
{
    return std::make_unique<FullColorLEDPattern>(
        std::vector<FullColorLEDPatternItem>{
            FullColorLEDPatternItem{.ticks = 3 * 2, .color = STANDARD_COLOR},
            FullColorLEDPatternItem{.ticks = 12 * 2, .color = DEFAULT_COLOR},
            FullColorLEDPatternItem{.ticks = 15 * 2, .color = STANDARD_COLOR},
            FullColorLEDPatternItem{.ticks = 120 * 2, .color = DEFAULT_COLOR},
        },
        0);
}

static std::unique_ptr<FullColorLEDPattern> MakeStandardConnectedPattern()
{
    return std::make_unique<FullColorLEDPattern>(
        std::vector<FullColorLEDPatternItem>{
            FullColorLEDPatternItem{.ticks = 18 * 2, .color = MODE1_COLOR},
            FullColorLEDPatternItem{.ticks = 30 * 2, .color = DEFAULT_COLOR},
        },
        3);
}

static std::unique_ptr<FullColorLEDPattern> MakeConfigAdvertizingPattern()
{
    return std::make_unique<FullColorLEDPattern>(
        std::vector<FullColorLEDPatternItem>{
            FullColorLEDPatternItem{.ticks = 3 * 2, .color = CONFIG_COLOR},
            FullColorLEDPatternItem{.ticks = 12 * 2, .color = DEFAULT_COLOR},
            FullColorLEDPatternItem{.ticks = 15 * 2, .color = CONFIG_COLOR},
            FullColorLEDPatternItem{.ticks = 120 * 2, .color = DEFAULT_COLOR},
        },
        0);
}

static std::unique_ptr<FullColorLEDPattern> MakeConfigConnectedPattern()
{
    return std::make_unique<FullColorLEDPattern>(
        std::vector<FullColorLEDPatternItem>{
            FullColorLEDPatternItem{.ticks = 18 * 2, .color = CONFIG_COLOR},
            FullColorLEDPatternItem{.ticks = 29 * 2, .color = DEFAULT_COLOR},
            FullColorLEDPatternItem{.ticks = 30 * 2, .color = CONFIG_COLOR},
        },
        3);
}

FullColorLEDIndicator::FullColorLEDIndicator(const std::array<uint8_t, 3> &pins) : m_pins(pins) {}

void FullColorLEDIndicator::InitPins()
{
    pinMode(m_pins[0], OUTPUT);
    pinMode(m_pins[1], OUTPUT);
    pinMode(m_pins[2], OUTPUT);
}

void FullColorLEDIndicator::StartConfigMode()
{
    m_pattern = MakeConfigAdvertizingPattern();
    m_mode = Mode::Config;
}

void FullColorLEDIndicator::StartStandardMode()
{
    m_pattern = MakeStandardAdvertizingPattern();
    m_mode = Mode::Standard;
}

void FullColorLEDIndicator::Connected()
{
    if (m_mode == Mode::Standard)
    {
        m_pattern = MakeStandardConnectedPattern();
    }
    else
    {
        m_pattern = MakeConfigConnectedPattern();
    }
}

void FullColorLEDIndicator::Disconnected()
{
    if (m_mode == Mode::Standard)
    {
        m_pattern = MakeStandardAdvertizingPattern();
    }
    else
    {
        m_pattern = MakeConfigAdvertizingPattern();
    }
}

void FullColorLEDIndicator::Tick()
{
    if (m_pattern == nullptr)
    {
        return;
    }
    auto result = m_pattern->Tick();
    if (!std::get<0>(result))
    {
        return;
    }
    auto color = std::get<1>(result);

    analogWrite(m_pins[0], 255 - color.r);
    analogWrite(m_pins[1], 255 - color.g);
    analogWrite(m_pins[2], 255 - color.b);
}