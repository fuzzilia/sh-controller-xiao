#ifndef SH_JOYCON_SHCONTROLLER_H
#define SH_JOYCON_SHCONTROLLER_H

#include "SHConfig.h"

typedef bool (*ReadButtonIsOn)(int button_index);

typedef float (*ReadStickValue)(int button_index, TwoDimension dimension);

struct MotionSensorValue {
    /**
     * このデータ1フレームあたりの秒数(秒)
     */
    float time_span_s;

    /**
     * ジャイロセンサーの値 (1回転毎秒を1とした角速度)
     */
    ThreeDimensionValue<float> gyro;
};

typedef std::vector<MotionSensorValue> (*ReadMotionSensorValues)();

class SHController {
    class ProcessRotationState {
        int8_t m_last_direction;
        int8_t m_first_direction;
        float m_last_rotate;
        int m_count;

    public:
        void Reset();

        int ProcessWithRelativeRotate(float rotate);

        static NullableThreeDimension LockedAxis(const ThreeDimensionValue<ProcessRotationState> &states);
    };

    struct RotateMotionState {
        NullableThreeDimension locked_axis;
        ThreeDimensionValue<ProcessRotationState> rotation_state;
    };

    struct MotionGestureState {
        // TODO メンバ変数を定義
    };

    struct StickRotationState {
        ProcessRotationState rotation;
        float last_angle;
        bool is_in_center_area;
    };

    union State {
        RotateMotionState rotate_motion;
        MotionGestureState motion_gesture;
        StickRotationState stick_rotation;
    };

    std::unique_ptr<SHConfig> m_config;
    ReadButtonIsOn m_read_button_is_on;
    ReadStickValue m_read_stick_value;
    ReadMotionSensorValues m_read_motion_sensor_values;
    State m_state;
    KeyboardValue m_last_keyboard_value;
    uint8_t m_last_combination_state = 0;
    uint8_t m_last_button_number = 0;

public:
    SHController(
            std::unique_ptr<SHConfig> config,
            ReadButtonIsOn read_button_is_on,
            ReadStickValue read_stick_value,
            ReadMotionSensorValues m_read_motion_sensor_values) :
            m_config(std::move(config)),
            m_read_button_is_on(read_button_is_on),
            m_read_stick_value(read_stick_value),
            m_read_motion_sensor_values(m_read_motion_sensor_values) {}

    std::vector<KeyboardValue> tick();

    const SHConfig &config() const;

private:
    void ProcessButtonRotation(std::vector<KeyboardValue> &keys, const SHConfig::RotateButtonConfig &config);

    void ProcessButtonGesture(std::vector<KeyboardValue> &keys, const SHConfig::GestureButtonConfig &config);
};


#endif //SH_JOYCON_SHCONTROLLER_H
