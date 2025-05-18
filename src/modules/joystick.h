#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <modules/button.h>

#define _OPTIMIZED (1)

namespace module
{
    /**
     * @brief ジョイスティックの軸を表すクラス
     * @tparam pin     ピン番号
     * @tparam inverse 数値を反転させるか(true:反転,false:通常)
     */
    template <uint8_t pin, bool inverse=false>
    class axis
    {
    public:
        axis() = delete;

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
            analogReadResolution(10);
            pinMode(pin, INPUT);
        }


        /**
         * @brief 軸のADC値を取得する
         * @return 軸の値
         */
        constexpr static uint32_t getValue()
        {
            if (inverse) {
                return 1024 - analogRead(pin);
            }
            else {
                return analogRead(pin);
            }
        }
    };


    /**
     * @brief ジョイスティックを表すクラス
     * @tparam _xAxis    X軸を表すaxisクラス
     * @tparam _yAxis    Y軸を表すaxisクラス
     * @tparam buttonPin 押し込みボタンのピン番号
     */
    template <typename _xAxis, typename _yAxis, uint8_t buttonPin>
    class joystick
    {
    public:
        using xAxis = _xAxis;
        using yAxis = _yAxis;
        using pushButton = button<buttonPin>;


        /**
         * @brief ピンをアサインする
         */
        constexpr static void assign()
        {
            xAxis::assign();
            yAxis::assign();
            pushButton::assign();
        }


        /**
         * @brief X軸の値を取得
         * @return X軸の値
         */
        constexpr static uint32_t getX()
        {
            return xAxis::getValue();
        }


        /**
         * @brief Y軸の値を取得
         * @return Y軸の値
         */
        constexpr static uint32_t getY()
        {
            return yAxis::getValue();
        }


        /**
         * @brief ボタンが押下されているか
         * @retval true  押下
         * @retval false 未押下
         */
        constexpr static bool isButtonPressed()
        {
            return pushButton::isPressed();
        }
    };
}