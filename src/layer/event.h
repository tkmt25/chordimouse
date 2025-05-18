#pragma once

#include <stdint.h>

enum Input 
{
    BUTTON_1 = 0x0001,
    BUTTON_2 = 0x0002,
    BUTTON_3 = 0x0004,
    BUTTON_4 = 0x0008,
    MIDDLE_BUTTON_1 = 0x0010,
    JOYSTICK_BUTTON = 0x0020,
    JOYSTICK_UP = 0x0040,
    JOYSTICK_DOWN = 0x0080,
    JOYSTICK_LEFT = 0x0100,
    JOYSTICK_RIGHT = 0x0200,
};

enum Event
{
    PRESS = 0x0001,
    RELEASE = 0x0002,
};


