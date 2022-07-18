#include "SHController.h"
#include <cmath>

#define MAX_KEY_NUM 32

static void PushKeys(std::queue<KeyboardValue> &keys, KeyboardValue key) {
    if (keys.size() < MAX_KEY_NUM - 1) {
        keys.push(key);
    }
}

static void PushKeys(std::queue<KeyboardValue> &keys, KeyboardValue key1, KeyboardValue key2) {
    if (keys.size() < MAX_KEY_NUM - 2) {
        keys.push(key1);
        keys.push(key2);
    }
}

void SHController::ProcessRotationState::Reset() {
    m_last_direction = 0;
    m_first_direction = 0;
    m_last_rotate = 0;
    m_count = 0;
}

int SHController::ProcessRotationState::ProcessWithRelativeRotate(float rotate) {
    const auto current_rotate = m_last_rotate + rotate;
    float rotate_offset = 0;
    if ((m_first_direction == 0 && current_rotate < 0) || m_first_direction == -1) {
        rotate_offset = 0.99999999;
    }

    const float rotate_diff_from_split_point = current_rotate + rotate_offset - (float) m_count;
    int count = (int) floorf(current_rotate + rotate_offset);
    if ((m_last_direction == -1 && rotate_diff_from_split_point > 0 && rotate_diff_from_split_point < 1.25f) ||
        (m_last_direction == 1 && rotate_diff_from_split_point < 0 && rotate_diff_from_split_point > -1.25f)) {
        count = m_count;
    }
    const int count_diff = count - m_count;
    int8_t direction = 0;
    if (count_diff > 0) {
        direction = 1;
    } else if (count_diff < 0) {
        direction = -1;
    }

    m_last_rotate = current_rotate;
    m_first_direction = m_first_direction != 0 ? m_first_direction : direction;
    m_last_direction = direction;
    m_count = count;
    return count_diff;
}

void
ProcessCountDiff(std::queue<KeyboardValue> &keys, int count_diff, const PositiveAndNegative<KeyboardValue> &config) {
    if (config.positive) {
        for (int i = 0; i < count_diff; i++) {
            PushKeys(keys, config.positive, KeyboardValue());
        }
    }
    if (config.negative) {
        for (int i = 0; i < -count_diff; i++) {
            PushKeys(keys, config.negative, KeyboardValue());
        }
    }
}

NullableThreeDimension
SHController::ProcessRotationState::LockedAxis(const ThreeDimensionValue<ProcessRotationState> &states) {
    const float absX = abs(states.x.m_last_rotate);
    const float absY = abs(states.y.m_last_rotate);
    const float absZ = abs(states.z.m_last_rotate);
    const float max = std::max(std::max(absX, absY), absZ);
    if (max < 1) {
        return NullableThreeDimension::Null;
    }
    if (absX == max) {
        return NullableThreeDimension::X;
    }
    if (absY == max) {
        return NullableThreeDimension::Y;
    }
    return NullableThreeDimension::Z;
}

void SHController::tick(std::queue<KeyboardValue> &keys) {
    uint8_t combination_state = 0;
    for (uint8_t button_index = 0; button_index < m_config->CombinationButtonNumbers().size(); button_index++) {
        if (m_read_button_is_on(m_config->CombinationButtonNumbers()[button_index])) {
            combination_state |= 0x01 << button_index;
        }
    }

    SHConfig::ButtonBlock button;
    uint8_t button_number = 0;
    for (uint8_t button_index = 0; button_index < m_config->StandardButtonNumbers().size(); button_index++) {
        if (m_read_button_is_on(m_config->StandardButtonNumbers()[button_index])) {
            button = m_config->GetButtonBlock(combination_state, button_index);
            if (button.BlockType() != ButtonBlockType::Empty) {
                button_number = button_index + 1;
                break;
            }
        }
    }

    bool button_state_changed = false;

    if (combination_state != m_last_combination_state || button_number != m_last_button_number) {
        if (m_last_keyboard_value) {
            m_last_keyboard_value = KeyboardValue();
            PushKeys(keys, m_last_keyboard_value);
        }
        button_state_changed = true;
    }
    m_last_combination_state = combination_state;
    m_last_button_number = button_number;

    auto buttonBlockType = button.BlockType();
    if (buttonBlockType != ButtonBlockType::Empty) {
        switch (button.BlockType()) {
            case ButtonBlockType::Empty:
                break;

            case ButtonBlockType::Standard:
                if (button_state_changed) {
                    const auto keyboardValue = button.GetKeyboardValue();
                    m_last_keyboard_value = keyboardValue;
                    PushKeys(keys, keyboardValue);
                }
                return;

            case ButtonBlockType::Gesture:
                if (button_state_changed) {
                    // TODO MotionGestureStateの定義に合わせて初期化処理
                    m_state.motion_gesture = {};
                }
                return;

            case ButtonBlockType::Rotation:
                if (button_state_changed) {
                    m_state.rotate_motion = {
                            .locked_axis = NullableThreeDimension::Null,
                            .rotation_state = {
                                    .x = ProcessRotationState(),
                                    .y = ProcessRotationState(),
                                    .z = ProcessRotationState(),
                            }
                    };
                }
                ProcessButtonRotation(keys, m_config->RotateButtonConfigAt(button.ReferenceIndex()));
                return;
        }
    }

    uint16_t stick_config = 0;
    for (uint8_t stick_index = 0; stick_index < m_config->StickCount(); stick_index++) {
        const auto &stick = m_config->GetStickBlock(combination_state, stick_index);
        auto block_type = stick.BlockType();
        if (block_type == StickBlockType::Empty) {
            continue;
        }

        const auto x = m_read_stick_value(stick_index, TwoDimension::X);
        const auto y = m_read_stick_value(stick_index, TwoDimension::Y);
        const auto magnitude = x * x + y * y;
        if (magnitude <= 0.2) {
            switch (block_type) {
                case StickBlockType::Empty:
                    break;
                case StickBlockType::Rotate:
                    m_state.stick_rotation = {ProcessRotationState(), 0, true};
                    break;
                case StickBlockType::FourButton:
                case StickBlockType::EightButton:
                    if (m_last_keyboard_value) {
                        m_last_keyboard_value = KeyboardValue();
                        PushKeys(keys, m_last_keyboard_value);
                    }
                    break;
            }
            continue;
        }

        const auto angle = atan2f(y, x);
        // y軸の正方向を0、時計回りを正、一周あたり1となるように正規化
        const float normalized_angle = fmodf((float) 1.25f - angle * (float) M_1_PI * 0.5, 1.0f);

        switch (block_type) {
            case StickBlockType::FourButton: {
                const auto &config = m_config->FourButtonStickConfigAt(stick.ReferenceIndex());
                const long index = lroundf(normalized_angle * 4) % 4;
                const auto key = config.keys[index];
                if (key && key != m_last_keyboard_value) {
                    PushKeys(keys, key);
                    m_last_keyboard_value = key;
                    return;
                }
                break;
            }

            case StickBlockType::EightButton: {
                const auto &config = m_config->EightButtonStickConfigAt(stick.ReferenceIndex());
                const long index = lroundf(normalized_angle * 8) % 8;
                const auto key = config.keys[index];
                if (key && key != m_last_keyboard_value) {
                    PushKeys(keys, key);
                    m_last_keyboard_value = key;
                    return;
                }
                break;
            }

            case StickBlockType::Rotate: {
                const auto &config = m_config->RotateStickConfigAt(stick.ReferenceIndex());
                if (button_state_changed) {
                    m_state.stick_rotation = {ProcessRotationState(), normalized_angle, false};
                    m_last_keyboard_value = KeyboardValue();
                } else if (m_state.stick_rotation.is_in_center_area) {
                    m_state.stick_rotation = {ProcessRotationState(), normalized_angle, false};
                    break;
                }
                float normalized_angle_diff = normalized_angle - m_state.stick_rotation.last_angle;
                m_state.stick_rotation.last_angle = normalized_angle;
                if (normalized_angle_diff < -0.5) {
                    normalized_angle_diff += 1;
                } else if (normalized_angle_diff > 0.5) {
                    normalized_angle_diff -= 1;
                }
                int count_diff = m_state.stick_rotation.rotation.ProcessWithRelativeRotate(
                        normalized_angle_diff * config.split_size);
                ProcessCountDiff(keys, count_diff, config.key);
                break;
            }

            default:
                break;
        }
    }

    return;
}

void SHController::ProcessButtonRotation(std::queue<KeyboardValue> &keys, const SHConfig::RotateButtonConfig &config) {
    auto sensor_values = m_read_motion_sensor_values();
    ThreeDimensionValue<float> adding_rotation = {.x = 0, .y = 0, .z = 0};
    auto locked_axis = m_state.rotate_motion.locked_axis;
    auto axis_is_locked = locked_axis != NullableThreeDimension::Null;
    bool xIsActive = !axis_is_locked || locked_axis == NullableThreeDimension::X;
    bool yIsActive = !axis_is_locked || locked_axis == NullableThreeDimension::Y;
    bool zIsActive = !axis_is_locked || locked_axis == NullableThreeDimension::Z;
    for (const auto &sensor_value : sensor_values) {
        if (xIsActive) {
            adding_rotation.x += sensor_value.gyro.x * config.rotate_split_size.x * sensor_value.time_span_s;
        }
        if (yIsActive) {
            adding_rotation.y += sensor_value.gyro.y * config.rotate_split_size.y * sensor_value.time_span_s;
        }
        if (zIsActive) {
            adding_rotation.z += sensor_value.gyro.z * config.rotate_split_size.z * sensor_value.time_span_s;
        }
    }
    int x_diff = xIsActive ? m_state.rotate_motion.rotation_state.x.ProcessWithRelativeRotate(adding_rotation.x) : 0;
    int y_diff = yIsActive ? m_state.rotate_motion.rotation_state.y.ProcessWithRelativeRotate(adding_rotation.y) : 0;
    int z_diff = zIsActive ? m_state.rotate_motion.rotation_state.z.ProcessWithRelativeRotate(adding_rotation.z) : 0;

    NullableThreeDimension next_locked_axis = locked_axis;
    if (next_locked_axis == NullableThreeDimension::Null && config.locks_axis) {
        next_locked_axis = ProcessRotationState::LockedAxis(m_state.rotate_motion.rotation_state);
        switch (next_locked_axis) {
            case NullableThreeDimension::X:
                m_state.rotate_motion.rotation_state.y.Reset();
                y_diff = 0;
                m_state.rotate_motion.rotation_state.z.Reset();
                z_diff = 0;
                break;
            case NullableThreeDimension::Y:
                m_state.rotate_motion.rotation_state.x.Reset();
                x_diff = 0;
                m_state.rotate_motion.rotation_state.z.Reset();
                z_diff = 0;
                break;
            case NullableThreeDimension::Z:
                m_state.rotate_motion.rotation_state.x.Reset();
                x_diff = 0;
                m_state.rotate_motion.rotation_state.y.Reset();
                y_diff = 0;
                break;
            case NullableThreeDimension::Null:
                break;
        }
    }

    ProcessCountDiff(keys, x_diff, config.rotate.x);
    ProcessCountDiff(keys, y_diff, config.rotate.y);
    ProcessCountDiff(keys, z_diff, config.rotate.z);
}

void SHController::ProcessButtonGesture(std::queue<KeyboardValue> &keys, const SHConfig::GestureButtonConfig &config) {
    // TODO 実装
}

const SHConfig &SHController::config() const {
    return *m_config;
}
