#include "SHConfig.h"

#include <algorithm>
#include <sstream>
#include <bitset>

PositiveAndNegative<KeyboardValue> zeroPositiveOrNegative = {
        .positive = KeyboardValue(),
        .negative = KeyboardValue(),
};

class DataReader {
    const uint8_t *m_data;
    size_t m_current_index = 0;
public:
    DataReader(const uint8_t *data) : m_data(data) {}

    uint8_t ReadUint8() {
        return m_data[m_current_index++];
    }

    uint16_t ReadUint16() {
        uint16_t ret = m_data[m_current_index] | (m_data[m_current_index + 1] << 8);
        m_current_index += 2;
        return ret;
    }
};

static uint8_t ButtonCountForKeypad(KeypadId id) {
    switch (id) {
        case KeypadId::JoyConL:
        case KeypadId::JoyConR:
            return 11;
        case KeypadId::ShControllerNrf52:
            return 10;
        default:
            return 0;
    }
}

static uint8_t StickCountForKeypad(KeypadId id) {
    switch (id) {
        case KeypadId::JoyConL:
        case KeypadId::JoyConR:
            return 1;
        case KeypadId::ShControllerNrf52:
            return 1;
        default:
            return 0;
    }
}

static PositiveAndNegative<KeyboardValue> ReadPositiveAndNegative(DataReader &reader) {
    auto modifier = reader.ReadUint8();
    return {
            .positive = KeyboardValue(modifier & 0x0f, reader.ReadUint8()),
            .negative = KeyboardValue(modifier >> 4, reader.ReadUint8()),
    };
}

const KeyboardValue::ValueType &KeyboardValue::Value() const {
    return m_value;
}

KeyboardValue::KeyboardValue(uint8_t modifier, uint8_t key_code) : m_value({modifier, key_code}) {}

KeyboardValue::KeyboardValue() : m_value({0, 0}) {}

KeyboardValue::operator bool() const {
    return m_value[0] != 0 || m_value[1] != 0;
}

const uint8_t *KeyboardValue::RawValue() const {
    return m_value.data();
}

void KeyboardValue::WriteLabelToStream(std::ostringstream &stream) const {
    if (m_value[0]) {
        bool is_first = true;
        stream << "[";
        if (hasCtrl()) {
            stream << "Ctrl";
            is_first = false;
        }
        if (hasShift()) {
            if (!is_first) {
                stream << "/";
            }
            stream << "Shift";
            is_first = false;
        }
        if (hasAlt()) {
            if (!is_first) {
                stream << "/";
            }
            stream << "Alt";
            is_first = false;
        }
        if (hasGui()) {
            if (!is_first) {
                stream << "/";
            }
            stream << "Gui";
        }
        stream << "]";
    }
    stream << (int) m_value[1];
}

size_t KeyboardValue::RawValueSize() const {
    return m_value.size();
}

bool KeyboardValue::operator==(const KeyboardValue &another) const {
    return this->m_value[0] == another.m_value[0] && this->m_value[1] == another.m_value[1];
}

SHConfig::ButtonBlock::ButtonBlock() : m_value(0) {}

SHConfig::ButtonBlock::ButtonBlock(uint16_t value) : m_value(value) {}

ButtonBlockType SHConfig::ButtonBlock::BlockType() const {
    return (ButtonBlockType) (m_value >> 12);
}

SHConfig::ButtonBlock SHConfig::ButtonBlock::StandardButtonBlock(uint8_t modifier, uint8_t key_code) {
    return SHConfig::ButtonBlock(((int) ButtonBlockType::Standard << 12) | (modifier << 8) | key_code);
}

SHConfig::ButtonBlock SHConfig::ButtonBlock::ReferenceButtonBlock(ButtonBlockType type, uint8_t index) {
    return SHConfig::ButtonBlock(((int) type << 12) | index);
}

uint8_t SHConfig::ButtonBlock::ReferenceIndex() const {
    return m_value & 0x00FF;
}

KeyboardValue SHConfig::ButtonBlock::GetKeyboardValue() const {
    return KeyboardValue((m_value >> 8) & 0x0F, m_value & 0xFF);
}

SHConfig::StickBlock::StickBlock() : m_value(0) {}

SHConfig::StickBlock::StickBlock(uint16_t value) : m_value(value) {}

SHConfig::StickBlock SHConfig::StickBlock::ReferenceStickBlock(StickBlockType type, uint8_t index) {
    return StickBlock(((int) type << 12) | index);
}

StickBlockType SHConfig::StickBlock::BlockType() const {
    return (StickBlockType) (m_value >> 12);
}

uint8_t SHConfig::StickBlock::ReferenceIndex() const {
    return m_value & 0x00FF;
}

SHConfig::SHConfig(const uint8_t *data, const std::vector<KeypadId> &keypadIds) {
    DataReader reader(data);
    m_needsSensorInput = false;

    auto version = reader.ReadUint16();
    if (version != 1) {
        m_error = Error::UnknownVersion;
        return;
    }

    auto rawKeypadId = reader.ReadUint16();
    auto keypadIdIsValid = std::any_of(keypadIds.begin(), keypadIds.end(), [&rawKeypadId](KeypadId keypadId) {
        return (uint16_t)keypadId == rawKeypadId;
    });
    if (!keypadIdIsValid) {
        m_error = Error::UnknownKeypadId;
        return;
    }
    m_keypad_id = (KeypadId) rawKeypadId;
    uint8_t button_count = ButtonCountForKeypad(m_keypad_id);
    m_stick_count = StickCountForKeypad(m_keypad_id);

    {
        uint8_t combination_button_numbers[3];
        for (int i = 0; i < 3; i++) {
            combination_button_numbers[i] = reader.ReadUint8();
        }
        // 組み合わせボタンのインデックスをビットで表現する。(通常ボタンの列を作るときのため)
        // ボタン数が32を上回る場合は正しく動作しないので注意
        uint32_t is_combination = 0;
        for (int i = 0; i < 3; i++) {
            if (combination_button_numbers[i] >= button_count) {
                break;
            }
            is_combination |= 0x01 << combination_button_numbers[i];
            m_combination_button_numbers.push_back(combination_button_numbers[i]);
        }

        for (int i = 0; i < button_count; i++) {
            if (!(is_combination & (0x01 << i))) {
                m_standard_button_numbers.push_back(i);
            }
        }
    }
    auto standard_button_size = m_standard_button_numbers.size();

    uint8_t combination_size = 0x01 << m_combination_button_numbers.size();
    for (int combination = 0; combination < combination_size; combination++) {
        auto combination_header = reader.ReadUint8();
        bool button_exists = combination_header & 0x01;
        bool stick_exists = combination_header & 0x02;
        ConfigsForCombination config_for_combination;

        if (button_exists) {
            config_for_combination.buttons = new ButtonBlock[standard_button_size];

            for (int button_index = 0; button_index < standard_button_size; button_index++) {
                uint8_t header = reader.ReadUint8();
                switch ((ButtonBlockType) (header & 0x0f)) {
                    case ButtonBlockType::Empty:
                        button_index += header >> 4;
                        break;
                    case ButtonBlockType::Standard: {
                        uint8_t key = reader.ReadUint8();
                        config_for_combination.buttons[button_index] =
                                ButtonBlock::StandardButtonBlock(header >> 4, key);
                        break;
                    }
                    case ButtonBlockType::Gesture: {
                        m_needsSensorInput = true;
                        uint8_t index = m_gesture_button_configs.size();
                        bool x_is_active = header & 0x01;
                        bool y_is_active = header & 0x02;
                        bool z_is_active = header & 0x04;
                        GestureButtonConfig config = {
                                .rotate = {
                                        .x = x_is_active ? ReadPositiveAndNegative(reader)
                                                         : zeroPositiveOrNegative,
                                        .y = y_is_active ? ReadPositiveAndNegative(reader)
                                                         : zeroPositiveOrNegative,
                                        .z = z_is_active ? ReadPositiveAndNegative(reader)
                                                         : zeroPositiveOrNegative,
                                }
                        };
                        m_gesture_button_configs.push_back(std::move(config));
                        config_for_combination.buttons[button_index] =
                                ButtonBlock::ReferenceButtonBlock(ButtonBlockType::Gesture, index);
                        break;
                    }
                    case ButtonBlockType::Rotation: {
                        m_needsSensorInput = true;
                        uint8_t index = m_rotate_button_configs.size();
                        ThreeDimensionValue<uint8_t> split_size = {
                                .x = reader.ReadUint8(),
                                .y = reader.ReadUint8(),
                                .z = reader.ReadUint8(),
                        };
                        RotateButtonConfig config = {
                                .locks_axis = (header & 0x10) != 0,
                                .rotate_split_size = split_size,
                                .rotate = {
                                        .x = split_size.x > 0 ? ReadPositiveAndNegative(reader)
                                                              : zeroPositiveOrNegative,
                                        .y = split_size.y > 0 ? ReadPositiveAndNegative(reader)
                                                              : zeroPositiveOrNegative,
                                        .z = split_size.z > 0 ? ReadPositiveAndNegative(reader)
                                                              : zeroPositiveOrNegative,
                                }
                        };
                        m_rotate_button_configs.push_back(std::move(config));
                        config_for_combination.buttons[button_index] =
                                ButtonBlock::ReferenceButtonBlock(ButtonBlockType::Rotation, index);
                        break;
                    }
                    default:
                        m_error = Error::InvalidButtonBlock;
                        break;
                }
            }
        }

        if (stick_exists) {
            config_for_combination.sticks = new StickBlock[m_stick_count];
            for (int stick_index = 0; stick_index < m_stick_count; stick_index++) {
                uint8_t header = reader.ReadUint8();
                switch ((StickBlockType) (header & 0x0f)) {
                    case StickBlockType::Empty:
                        break;
                    case StickBlockType::Rotate: {
                        uint8_t index = m_rotate_stick_configs.size();
                        uint8_t split_size = (header >> 4) << 2;
                        RotateStickConfig config = {
                                .split_size = split_size,
                                .key = ReadPositiveAndNegative(reader),
                        };
                        m_rotate_stick_configs.push_back(std::move(config));
                        config_for_combination.sticks[stick_index] =
                                StickBlock::ReferenceStickBlock(StickBlockType::Rotate, index);
                        break;
                    }
                    case StickBlockType::FourButton: {
                        uint8_t index = m_four_button_stick_configs.size();
                        uint8_t key_modifiers[] = {reader.ReadUint8(), reader.ReadUint8()};
                        FourButtonStickConfig config;
                        for (int i = 0; i < 2; i++) {
                            config.keys[i * 2] = KeyboardValue(key_modifiers[i] & 0x0f, reader.ReadUint8());
                            config.keys[i * 2 + 1] = KeyboardValue(key_modifiers[i] >> 4, reader.ReadUint8());
                        }
                        m_four_button_stick_configs.push_back(std::move(config));
                        config_for_combination.sticks[stick_index] =
                                StickBlock::ReferenceStickBlock(StickBlockType::FourButton, index);
                        break;
                    }
                    case StickBlockType::EightButton: {
                        uint8_t index = m_eight_button_stick_configs.size();
                        uint8_t key_modifiers[] = {reader.ReadUint8(), reader.ReadUint8(),
                                                   reader.ReadUint8(), reader.ReadUint8()};
                        EightButtonStickConfig config;
                        for (int i = 0; i < 4; i++) {
                            config.keys[i * 2] = KeyboardValue(key_modifiers[i] & 0x0f, reader.ReadUint8());
                            config.keys[i * 2 + 1] = KeyboardValue(key_modifiers[i] >> 4, reader.ReadUint8());
                        }
                        m_eight_button_stick_configs.push_back(std::move(config));
                        config_for_combination.sticks[stick_index] =
                                StickBlock::ReferenceStickBlock(StickBlockType::EightButton, index);
                        break;
                    }
                }
            }
        }

        m_configs_by_combination.push_back(std::move(config_for_combination));
    }
    m_error = Error::None;
}

SHConfig::SHConfig(KeypadId keyapdId): m_error(Error::None), m_keypad_id(keyapdId), m_stick_count(1), m_needsSensorInput(false) {
    m_standard_button_numbers.push_back(0);
    m_standard_button_numbers.push_back(1);
    m_standard_button_numbers.push_back(2);

    ConfigsForCombination configs;
    configs.buttons = new ButtonBlock[3];
    configs.buttons[0] = ButtonBlock::StandardButtonBlock(0x00, 0x04); // a
    configs.buttons[1] = ButtonBlock::StandardButtonBlock(0x00, 0x05); // b
    configs.buttons[2] = ButtonBlock::StandardButtonBlock(0x00, 0x06); // c
    m_configs_by_combination.push_back(std::move(configs));
}

std::string SHConfig::ToString() const {
    std::ostringstream stream;

    stream << "Combination Buttons:";
    if (m_combination_button_numbers.empty()) {
        stream << " None\r\n\r\n";
    } else {
        stream << "\r\n";
        for (const auto number : m_combination_button_numbers) {
            stream << "  " << number << "\r\n";
        }
        stream << "\r\n";
    }

    for (unsigned combination_index = 0; combination_index < m_configs_by_combination.size(); combination_index++) {
        auto &for_combination = m_configs_by_combination[combination_index];
        stream << "Combination " << std::bitset<3>{combination_index} << ":\r\n";

        stream << "  Buttons:";
        if (for_combination.buttons) {
            stream << "\r\n";
            for (unsigned i = 0; i < m_standard_button_numbers.size(); i++) {
                stream << "    " << (int) m_standard_button_numbers[i] << " : ";
                auto &button = for_combination.buttons[i];
                switch (button.BlockType()) {
                    case ButtonBlockType::Empty:
                        stream << "Empty\r\n";
                        break;
                    case ButtonBlockType::Standard:
                        stream << "Standard ";
                        button.GetKeyboardValue().WriteLabelToStream(stream);
                        stream << "\r\n";
                        break;
                    case ButtonBlockType::Gesture: {
                        auto gesture = GestureButtonConfigAt(button.ReferenceIndex());
                        stream << "Gesture ";
                        stream << "\r\n";
                        stream << "      x+ : ";
                        gesture.rotate.x.positive.WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      x- : ";
                        gesture.rotate.x.negative.WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      y+ : ";
                        gesture.rotate.y.positive.WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      y- : ";
                        gesture.rotate.y.negative.WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      z+ : ";
                        gesture.rotate.z.positive.WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      z- : ";
                        gesture.rotate.z.negative.WriteLabelToStream(stream);
                        stream << "\r\n";
                        break;
                    }
                    case ButtonBlockType::Rotation: {
                        auto rotation = RotateButtonConfigAt(button.ReferenceIndex());
                        stream << "Rotation [lock:" << (rotation.locks_axis ? "enable" : "disable") << "\r\n";
                        stream << "      x : split-" << (int) rotation.rotate_split_size.x << "\r\n";
                        stream << "      x+ : ";
                        rotation.rotate.x.positive.WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      x- : ";
                        rotation.rotate.x.negative.WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      y : split-" << (int) rotation.rotate_split_size.y << "\r\n";
                        stream << "      y+ : ";
                        rotation.rotate.y.positive.WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      y- : ";
                        rotation.rotate.y.negative.WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      z : split-" << (int) rotation.rotate_split_size.z << "\r\n";
                        stream << "      z+ : ";
                        rotation.rotate.z.positive.WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      z- : ";
                        rotation.rotate.z.negative.WriteLabelToStream(stream);
                        stream << "\r\n";
                        break;
                    }
                }
            }
        } else {
            stream << " Empty\r\n";
        }

        stream << "  Sticks:";
        if (for_combination.sticks) {
            stream << "\r\n";
            for (int i = 0; i < m_stick_count; i++) {
                stream << "    " << i << " : ";
                auto &stick = for_combination.sticks[i];
                switch (stick.BlockType()) {
                    case StickBlockType::Empty:
                        stream << "Empty\r\n";
                        break;
                    case StickBlockType::FourButton: {
                        stream << "4Button\r\n";
                        auto button = FourButtonStickConfigAt(stick.ReferenceIndex());
                        stream << "      Up    : ";
                        button.keys[0].WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      Right : ";
                        button.keys[1].WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      Down  : ";
                        button.keys[2].WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      Left  : ";
                        button.keys[3].WriteLabelToStream(stream);
                        stream << "\r\n";
                        break;
                    }
                    case StickBlockType::EightButton: {
                        stream << "8Button\r\n";
                        auto button = EightButtonStickConfigAt(stick.ReferenceIndex());
                        stream << "      Up         : ";
                        button.keys[0].WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      Up-Right   : ";
                        button.keys[1].WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      Right      : ";
                        button.keys[2].WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      Down-Right : ";
                        button.keys[3].WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      Down       : ";
                        button.keys[4].WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      Down-Left  : ";
                        button.keys[5].WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      Left       : ";
                        button.keys[6].WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      Up-Left    : ";
                        button.keys[7].WriteLabelToStream(stream);
                        stream << "\r\n";
                        break;
                    }
                    case StickBlockType::Rotate: {
                        auto rotate = RotateStickConfigAt(stick.ReferenceIndex());
                        stream << "Rotate [split=" << (int) rotate.split_size << "]\r\n";
                        stream << "      + : ";
                        rotate.key.positive.WriteLabelToStream(stream);
                        stream << "\r\n";
                        stream << "      - : ";
                        rotate.key.negative.WriteLabelToStream(stream);
                        stream << "\r\n";
                        break;
                    }
                }
            }
        } else {
            stream << " Empty\r\n";
        }
    }

    return stream.str();
}

std::unique_ptr<SHConfig> SHConfig::defaultConfig(KeypadId keypadId) {
    return std::unique_ptr<SHConfig>(new SHConfig(keypadId));
}