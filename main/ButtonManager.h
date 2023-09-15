#ifndef SH_CONTROLLER_BUTTON_MANAGER_H
#define SH_CONTROLLER_BUTTON_MANAGER_H

/**
 * @brief 指定された番号のボタンが押されているかどうかを返します。
 *
 * @param index ボタン番号 (0始まり)
 * @return true
 * @return false
 */
bool ButtonIsOn(int index);

/**
 * @brief ボタンのピン設定を行います。
 *
 */
void InitButtons();

/**
 * @brief 設定ボタンが押されているかどうかを返します。
 *
 * @return true
 * @return false
 */
bool ConfigButtonIsOn();

#endif // SH_CONTROLLER_BUTTON_MANAGER_H