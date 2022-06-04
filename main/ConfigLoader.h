#ifndef SH_CONTROLLER_CONFIG_LOADER_H
#define SH_CONTROLLER_CONFIG_LOADER_H

#include <memory>

#include "src/lib/SHConfig.h"

/**
 * キー設定のストレージへの保存と読み込みを行う関数を定義するクラスです。
 * ESP32-Arduinoに依存した実装になっています。
 */
class ConfigLoader {
public:
    static void save(uint8_t *data, size_t size);
    static std::unique_ptr<SHConfig> load();
    static const uint8_t *loadRaw();
};

#endif // SH_CONTROLLER_CONFIG_LOADER_H