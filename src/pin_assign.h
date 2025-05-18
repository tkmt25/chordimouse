#pragma once

#include <Arduino.h>

namespace gpio {
    constexpr int JOYSTICK_X        =  PIN_A1; ///< ジョイスティックX軸
    constexpr int JOYSTICK_Y        =  PIN_A0; ///< ジョイスティックY軸
    constexpr int JOYSTICK_BUTTON   =  D2; ///< ジョイスティックボタン
    //constexpr int JOYSTICK_BUTTON   =  D5; ///< ジョイスティックボタン
    
    constexpr int BUTTON_1        =  D6;  ///< ボタン1
    constexpr int BUTTON_2        =  D8;  ///< ボタン2
    constexpr int BUTTON_3        =  D9;  ///< ボタン3
    constexpr int BUTTON_4        =  D10; ///< ボタン4
    constexpr int MIDDLE_BUTTON_1 =  D7;  ///< 中央ボタン1
    
    //constexpr int MODE          =  D2; ///< BLE接続先切り替え(HIGH: PC, LOW: スマホ)
    constexpr int MODE          =  D5; ///< BLE接続先切り替え(HIGH: PC, LOW: スマホ)
    
    constexpr int UNUSED_1     =  D3; ///< 未使用
    constexpr int UNUSED_2     =  D4; ///< 未使用
}

