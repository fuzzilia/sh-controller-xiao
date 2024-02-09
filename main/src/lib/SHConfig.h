#ifndef SH_JOYCON_SHCONFIG_H
#define SH_JOYCON_SHCONFIG_H

#include <array>
#include <memory>
#include <stdint.h>
#include <vector>

#include "SHValue.h"

enum class ButtonBlockType : uint8_t {
    Empty = 0,
    Standard = 1,
    Gesture = 2,
    Rotation = 3,
};

enum class StickBlockType : uint8_t {
    Empty = 0,
    Rotate = 1,
    FourButton = 2,
    EightButton = 3,
};

enum class KeypadId : uint16_t {
    ShControllerNrf52 = 0x0001,
    ShControllerNrf52XiaoR = 0x0003,
    ShControllerNrf52XiaoL = 0x0004,
    ShControllerNrf52XiaoSenseR = 0x0005,
    ShControllerNrf52XiaoSenseL = 0x0006,
    JoyConL = 0x0081,
    JoyConR = 0x0082,
};

class KeyboardValue {
  public:
    typedef std::array<uint8_t, 2> ValueType;

    enum class Modifier : uint8_t {
        Ctrl = 0,
        Shift,
        Alt,
        Gui,
    };

  private:
    ValueType m_value;

  public:
    KeyboardValue();

    KeyboardValue(uint8_t modifier, uint8_t key_code);

    operator bool() const;

    const ValueType &Value() const;

    const uint8_t *RawValue() const;

    size_t RawValueSize() const;

    void WriteLabelToStream(std::ostringstream &stream) const;

    bool operator==(const KeyboardValue &another) const;

    inline bool operator!=(const KeyboardValue &rhs) const {
        return !(*this == rhs);
    }

    inline bool hasCtrl() const {
        return m_value[0] & (0x01 << (uint8_t)Modifier::Ctrl);
    }

    inline bool hasShift() const {
        return m_value[0] & (0x01 << (uint8_t)Modifier::Shift);
    }

    inline bool hasAlt() const {
        return m_value[0] & (0x01 << (uint8_t)Modifier::Alt);
    }

    inline bool hasGui() const {
        return m_value[0] & (0x01 << (uint8_t)Modifier::Gui);
    }

    inline uint8_t keyCode() const { return m_value[1]; }

    inline uint8_t modifier() const { return m_value[0]; }
};

class SHConfig {
  public:
    enum class Error : int {
        None,
        Uninitialized,
        UnknownVersion,
        UnknownKeypadId,
        InvalidButtonBlock,
    };

    class ButtonBlock {
        uint16_t m_value;

        explicit ButtonBlock(uint16_t value);

      public:
        ButtonBlock();

        static ButtonBlock StandardButtonBlock(uint8_t modifier,
                                               uint8_t key_code);

        static ButtonBlock ReferenceButtonBlock(ButtonBlockType type,
                                                uint8_t index);

        ButtonBlockType BlockType() const;

        KeyboardValue GetKeyboardValue() const;

        uint8_t ReferenceIndex() const;
    };

    class StickBlock {
        uint16_t m_value;

        explicit StickBlock(uint16_t value);

      public:
        StickBlock();

        static StickBlock ReferenceStickBlock(StickBlockType type,
                                              uint8_t index);

        StickBlockType BlockType() const;

        uint8_t ReferenceIndex() const;
    };

    struct GestureButtonConfig {
        ThreeDimensionValue<PositiveAndNegative<KeyboardValue>> rotate;
    };

    struct RotateButtonConfig {
        bool locks_axis;
        ThreeDimensionValue<uint8_t> rotate_split_size;
        ThreeDimensionValue<PositiveAndNegative<KeyboardValue>> rotate;
    };

    struct RotateStickConfig {
        uint8_t split_size;
        PositiveAndNegative<KeyboardValue> key;
    };

    struct FourButtonStickConfig {
        KeyboardValue keys[4];
    };

    struct EightButtonStickConfig {
        KeyboardValue keys[8];
    };

  private:
    SHConfig(KeypadId keyapdId);

    SHConfig(const SHConfig &) = delete;

    SHConfig &operator=(const SHConfig &) = delete;

    struct ConfigsForCombination {
        ButtonBlock *buttons = nullptr;
        StickBlock *sticks = nullptr;

        ~ConfigsForCombination() {
            delete[] buttons;
            delete[] sticks;
        }

        ConfigsForCombination() = default;

        ConfigsForCombination(ConfigsForCombination &&another) noexcept
            : buttons(another.buttons), sticks(another.sticks) {
            another.buttons = nullptr;
            another.sticks = nullptr;
        }
    };

    std::vector<ConfigsForCombination> m_configs_by_combination;
    Error m_error = Error::Uninitialized;
    KeypadId m_keypad_id;
    uint8_t m_stick_count;
    std::vector<uint8_t> m_combination_button_numbers;
    std::vector<uint8_t> m_standard_button_numbers;
    std::vector<GestureButtonConfig> m_gesture_button_configs;
    std::vector<RotateButtonConfig> m_rotate_button_configs;
    std::vector<RotateStickConfig> m_rotate_stick_configs;
    std::vector<FourButtonStickConfig> m_four_button_stick_configs;
    std::vector<EightButtonStickConfig> m_eight_button_stick_configs;
    bool m_needsSensorInput;

  public:
    SHConfig(const uint8_t *data, const std::vector<KeypadId> &keypadIds);

    static std::unique_ptr<SHConfig> defaultConfig(KeypadId keypadId);

    bool isValid() const { return m_error == Error::None; }

    Error error() const { return m_error; }

    const std::vector<uint8_t> &CombinationButtonNumbers() const {
        return m_combination_button_numbers;
    }

    const std::vector<uint8_t> &StandardButtonNumbers() const {
        return m_standard_button_numbers;
    }

    uint8_t StickCount() const { return m_stick_count; }

    ButtonBlock GetButtonBlock(uint8_t combination_index,
                               uint8_t button_index) const {
        auto buttons = m_configs_by_combination[combination_index].buttons;
        return buttons ? buttons[button_index] : ButtonBlock();
    }

    StickBlock GetStickBlock(uint8_t combination_index,
                             uint8_t stick_index) const {
        auto sticks = m_configs_by_combination[combination_index].sticks;
        return sticks ? sticks[stick_index] : StickBlock();
    }

    const GestureButtonConfig &GestureButtonConfigAt(uint8_t index) const {
        return m_gesture_button_configs[index];
    }

    const RotateButtonConfig &RotateButtonConfigAt(uint8_t index) const {
        return m_rotate_button_configs[index];
    }

    const RotateStickConfig &RotateStickConfigAt(uint16_t index) const {
        return m_rotate_stick_configs[index];
    }

    const FourButtonStickConfig &FourButtonStickConfigAt(uint16_t index) const {
        return m_four_button_stick_configs[index];
    }

    const EightButtonStickConfig &
    EightButtonStickConfigAt(uint16_t index) const {
        return m_eight_button_stick_configs[index];
    }

    bool NeedsSensorInput() const { return m_needsSensorInput; }

    KeypadId GetKeypadId() const { return m_keypad_id; }

    float StickRotateOffset(uint16_t index) const;

    std::string ToString() const;
};

#endif // SH_JOYCON_SHCONFIG_H
