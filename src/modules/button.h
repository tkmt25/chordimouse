#pragma once

#include <stdint.h>
#include <Arduino.h>

namespace module
{
    /**
     * @brief ボタンを表すクラス
     * 
     * @tparam pin ピン番号
     */
    template <uint8_t pin>
    class button
    {
    public:
        button() = delete;

        /**
         * @brief ピン番号を取得
         * @return ピン番号
         */
        constexpr static uint8_t getPin()
        {
            return pin;
        }


        /**
         * @brief ピンをアサインする
         */
        constexpr static void assign()
        {
            pinMode(pin, INPUT_PULLUP);
        }


        /**
         * @brief ボタンが押下されているか
         * @retval true  押下
         * @retval false 未押下
         */
        constexpr static bool isPressed()
        {
            return !digitalRead(pin);
        }
    };
}