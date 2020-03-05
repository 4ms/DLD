#pragma once
#include <stdint.h>

#ifndef __cplusplus
#define bool uint8_t
#endif

constexpr float SAMPLERATE = (48000.0f);
constexpr float one_volt = (float)(1<<15) / 12.2f;
constexpr float five_volts = one_volt * 5.0f; 
constexpr float zero_volts = -one_volt * 0.6f;
constexpr float neg_five_volts = -one_volt * 6.4f;

uint8_t hardwaretest_continue_button(void);
void pause_until_button_pressed(void);
void pause_until_button_released(void);
void flash_ping_until_pressed(void);

bool read_button_state(uint8_t button_num);
uint8_t read_switch_state(uint8_t sw_num);
void set_button_led(uint8_t button_num, bool turn_on);
void set_led(uint8_t led_num, bool turn_on);
void all_leds_on(void);
