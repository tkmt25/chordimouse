#pragma once

#include <pin_assign.h>
#include <modules/button.h>
#include <modules/joystick.h>
#include <modules/led_indicator.h>
#include <utils/sleep_controller.h>
#include <utils/click_detector.h>
#include <layer/keyboard_layer.h>
#include <layer/mouse_layer.h>


using button1 = module::button<gpio::BUTTON_1>;
using button2 = module::button<gpio::BUTTON_2>;
using button3 = module::button<gpio::BUTTON_3>;
using button4 = module::button<gpio::BUTTON_4>;
using middle_button1 = module::button<gpio::MIDDLE_BUTTON_1>;
using mode_sw = module::button<gpio::MODE>;
using xAxis = module::axis<gpio::JOYSTICK_X, true>;
using yAxis = module::axis<gpio::JOYSTICK_Y, true>;
using joystick = module::joystick<xAxis, yAxis, gpio::JOYSTICK_BUTTON>;
//using joystick = module::joystick<gpio::JOYSTICK_X, gpio::JOYSTICK_Y, gpio::JOYSTICK_BUTTON>;
using led_indicator = module::led_indicator<LED_RED, LED_GREEN, LED_BLUE>;

using keyboard_layer = layer::keyboard_layer<button1, button2, button3, button4, middle_button1, joystick>;
using mouse_layer = layer::mouse_layer<button1, button2, button3, button4, middle_button1, joystick>;
using sleep_controller = utils::sleep_controller<button1, button2, button3, button4, middle_button1, joystick>;
using click_detector = utils::click_detector<joystick::pushButton>;