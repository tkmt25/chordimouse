#pragma once

#include <bluefruit.h>
#include <ble/ble_common.h>

namespace ble
{
    class ble_hid
    {
    public:
        
        enum MouseButton
        {
            LEFT = MOUSE_BUTTON_LEFT,
            RIGHT = MOUSE_BUTTON_RIGHT,
            MIDDLE = MOUSE_BUTTON_MIDDLE,
            BACKWARD = MOUSE_BUTTON_BACKWARD,
            FORWARD = MOUSE_BUTTON_FORWARD,
        };

        enum Modifier
        {
            NONE = 0,
            LEFT_CTRL = KEYBOARD_MODIFIER_LEFTCTRL,
            LEFT_SHIFT = KEYBOARD_MODIFIER_LEFTSHIFT,
            LEFT_ALT = KEYBOARD_MODIFIER_LEFTALT,
            LEFT_GUI = KEYBOARD_MODIFIER_LEFTGUI,
            RIGHT_CTRL = KEYBOARD_MODIFIER_RIGHTCTRL,
            RIGHT_SHIFT = KEYBOARD_MODIFIER_RIGHTSHIFT,
            RIGHT_ALT = KEYBOARD_MODIFIER_RIGHTALT,
            RIGHT_GUI = KEYBOARD_MODIFIER_RIGHTGUI,
        };

        ble_hid() = delete;

        static void init();
        static bool connect(uint8_t peerId, uint32_t timeoutMs = 0, const ConnectionParam param = ConnectionParam());
        static void disconnect(const uint32_t timeoutMs = 0);
        static bool isConnected();

        static void mouseMove(const int8_t x, const int8_t y);
        static void mouseHScroll(const int8_t move);
        static void mouseVScroll(const int8_t move);
        static void mousePress(const MouseButton button);
        static void mouseRelease(const MouseButton button);
        
        static void keyPress(const uint8_t scancode, const uint8_t modifierFlag = Modifier::NONE);
        static void keyRelease(const uint8_t modifierFlag = Modifier::NONE);

    private:
        static inline BLEHidAdafruit blehid;
        static inline uint16_t connectionHandle = BLE_CONN_HANDLE_INVALID;
    };
}