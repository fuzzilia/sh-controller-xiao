#ifndef SH_CONTROLLER_INPUT_H
#define SH_CONTROLLER_INPUT_H

#include "src/lib/SHValue.h"

/**
 * @brief 指定された番号のボタンが押されているかどうかを返します。
 *
 * @param index ボタン番号 (0始まり)
 * @return true
 * @return false
 */
bool ButtonIsOn(int index);

/**
 * @brief 入力機能のための各種初期化を行います。
 */
void InitializeInput();

/**
 * @brief 設定ボタンが押されているかどうかを返します。
 *
 * @return true
 * @return false
 */
bool ConfigButtonIsOn();

void RefreshInput();

float StickValue(int, TwoDimension dimension);

#endif // SH_CONTROLLER_INPUT_H