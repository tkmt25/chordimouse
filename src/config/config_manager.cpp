#include <config/config_manager.h>
#include <layer/event.h>
#include <bluefruit.h>

//config config_manager::_config;
//key_profile config_manager::_keyProfiles[2];

// デフォルト設定(README.md参照)
config config_manager::DEFAULT_CONFIG;

// 0 : 通常
// 1 : Fnキー
key_profile config_manager::DEFAULT_KEY_PROFILES[2] = {
    // 通常
    {
        {Input::BUTTON_1,                                                   HID_KEY_Z},
        {Input::BUTTON_2,                                                   HID_KEY_C},
        {Input::BUTTON_3,                                                   HID_KEY_V},
        {Input::BUTTON_4,                                                   HID_KEY_S},
        {Input::MIDDLE_BUTTON_1,                                            HID_KEY_F},
        {Input::BUTTON_1 | Input::BUTTON_2,                                 HID_KEY_A},
        {Input::BUTTON_1 | Input::BUTTON_3,                                 HID_KEY_B},
        {Input::BUTTON_1 | Input::BUTTON_4,                                 HID_KEY_D},
        {Input::BUTTON_1 | Input::MIDDLE_BUTTON_1,                          HID_KEY_N},
        {Input::BUTTON_2 | Input::BUTTON_3,                                 HID_KEY_P},
        {Input::BUTTON_2 | Input::BUTTON_4,                                 HID_KEY_E},
        {Input::BUTTON_2 | Input::MIDDLE_BUTTON_1,                          HID_KEY_G},
        {Input::BUTTON_3 | Input::BUTTON_4,                                 HID_KEY_H},
        {Input::BUTTON_3 | Input::MIDDLE_BUTTON_1,                          HID_KEY_I},
        {Input::BUTTON_4 | Input::MIDDLE_BUTTON_1,                          HID_KEY_J},
        {Input::BUTTON_1 | Input::BUTTON_2 | Input::BUTTON_3,               HID_KEY_K},
        {Input::BUTTON_1 | Input::BUTTON_2 | Input::BUTTON_4,               HID_KEY_L},
        {Input::BUTTON_1 | Input::BUTTON_2 | Input::MIDDLE_BUTTON_1,        HID_KEY_M},
        {Input::BUTTON_1 | Input::BUTTON_3 | Input::BUTTON_4,               HID_KEY_O},
        {Input::BUTTON_1 | Input::BUTTON_3 | Input::MIDDLE_BUTTON_1,        HID_KEY_Q},
        {Input::BUTTON_1 | Input::BUTTON_4 | Input::MIDDLE_BUTTON_1,        HID_KEY_R},
        {Input::BUTTON_2 | Input::BUTTON_3 | Input::BUTTON_4,               HID_KEY_T},
        {Input::BUTTON_2 | Input::BUTTON_3 | Input::MIDDLE_BUTTON_1,        HID_KEY_U},
        {Input::BUTTON_2 | Input::BUTTON_4 | Input::MIDDLE_BUTTON_1,        HID_KEY_W},
        {Input::BUTTON_3 | Input::BUTTON_4 | Input::MIDDLE_BUTTON_1,        HID_KEY_X},
        {Input::BUTTON_1 | Input::BUTTON_2 | Input::BUTTON_3 | Input::BUTTON_4, HID_KEY_Y},
    },
    
    // Fnキー
    {
        {Input::BUTTON_1,                                                   HID_KEY_ENTER},
        {Input::BUTTON_2,                                                   HID_KEY_SPACE},
        {Input::BUTTON_3,                                                   HID_KEY_TAB},
        {Input::BUTTON_4,                                                   HID_KEY_BACKSPACE},
        {Input::MIDDLE_BUTTON_1,                                            HID_KEY_GRAVE}, // 半角/全角
        {Input::BUTTON_1 | Input::BUTTON_2,                                 HID_KEY_DELETE},
        {Input::BUTTON_1 | Input::BUTTON_3,                                 HID_KEY_NONE},
        {Input::BUTTON_1 | Input::BUTTON_4,                                 HID_KEY_NONE},
        {Input::BUTTON_1 | Input::MIDDLE_BUTTON_1,                          HID_KEY_NONE},
        {Input::BUTTON_2 | Input::BUTTON_3,                                 HID_KEY_NONE},
        {Input::BUTTON_2 | Input::BUTTON_4,                                 HID_KEY_NONE},
        {Input::BUTTON_2 | Input::MIDDLE_BUTTON_1,                          HID_KEY_NONE},
        {Input::BUTTON_3 | Input::BUTTON_4,                                 HID_KEY_NONE},
        {Input::BUTTON_3 | Input::MIDDLE_BUTTON_1,                          HID_KEY_NONE},
        {Input::BUTTON_4 | Input::MIDDLE_BUTTON_1,                          HID_KEY_NONE},
        {Input::BUTTON_1 | Input::BUTTON_2 | Input::BUTTON_3,               HID_KEY_NONE},
        {Input::BUTTON_1 | Input::BUTTON_2 | Input::BUTTON_4,               HID_KEY_NONE},
        {Input::BUTTON_1 | Input::BUTTON_2 | Input::MIDDLE_BUTTON_1,        HID_KEY_NONE},
        {Input::BUTTON_1 | Input::BUTTON_3 | Input::BUTTON_4,               HID_KEY_NONE},
        {Input::BUTTON_1 | Input::BUTTON_3 | Input::MIDDLE_BUTTON_1,        HID_KEY_NONE},
        {Input::BUTTON_1 | Input::BUTTON_4 | Input::MIDDLE_BUTTON_1,        HID_KEY_NONE},
        {Input::BUTTON_2 | Input::BUTTON_3 | Input::BUTTON_4,               HID_KEY_NONE},
        {Input::BUTTON_2 | Input::BUTTON_3 | Input::MIDDLE_BUTTON_1,        HID_KEY_NONE},
        {Input::BUTTON_2 | Input::BUTTON_4 | Input::MIDDLE_BUTTON_1,        HID_KEY_NONE},
        {Input::BUTTON_3 | Input::BUTTON_4 | Input::MIDDLE_BUTTON_1,        HID_KEY_NONE},
        {Input::BUTTON_1 | Input::BUTTON_2 | Input::BUTTON_3 | Input::BUTTON_4, HID_KEY_ESCAPE},
    }
};
