#include "hardware_test_util.h"
#include "dig_pins.h"
#include "leds.h"
#include "globals.h"

#include "adc.h"

extern uint16_t potadc_buffer[NUM_POT_ADCS];
extern uint16_t cvadc_buffer[NUM_CV_ADCS];

void setup_adc(void) {
	Deinit_Pot_ADC();
	Deinit_CV_ADC();

	Init_Pot_ADC((uint16_t *)potadc_buffer, NUM_POT_ADCS);
	Init_CV_ADC((uint16_t *)cvadc_buffer, NUM_CV_ADCS);
}

struct AdcCheck {
    const uint16_t center_val;
    const uint16_t center_width;
    const uint16_t min_val;
    const uint16_t max_val;
    const uint32_t center_check_rate; 
    uint16_t cur_val;
    uint32_t status;
};
//returns 1 if adc is fully ranged checked
uint8_t check_adc(struct AdcCheck *adc_check)
{
    if (adc_check->cur_val < adc_check->min_val) {
    	LED_LOOP1_OFF;
        adc_check->status &= ~(0b10UL);
    }
    if (adc_check->cur_val > adc_check->max_val) {
        LED_LOOP2_OFF;
        adc_check->status &= ~(0b01UL);
    }
    if (adc_check->cur_val>(adc_check->center_val - adc_check->center_width) \
        && adc_check->cur_val<(adc_check->center_val + adc_check->center_width)) {
        LED_PINGBUT_OFF;
        adc_check->status -= adc_check->center_check_rate; //count down
    }
    else {
        adc_check->status |= ~(0b11UL); //reset counter
        LED_PINGBUT_ON;
    }
    if ((adc_check->status & 0xFFFF0003)==0)
        return 1;
    else
        return 0;
}

void test_pots(void) {
	LED_LOOP1_ON;
	LED_LOOP2_ON;
	LED_PINGBUT_ON;
	LED_INF1_ON;
	LED_REV1_ON;

	setup_adc();
	LED_INF1_OFF;
	LED_REV1_OFF;

    struct AdcCheck adc_check = {
        .center_val = 2048,
        .center_width = 50,
        .min_val = 10,
        .max_val = 4000,
        .center_check_rate = (1UL<<15)
    };
    for (uint32_t adc_i=0; adc_i<NUM_POT_ADCS; adc_i++) {
        adc_check.status = 0xFFFFFFFF;
        LED_LOOP1_ON;
		LED_LOOP2_ON;
		LED_PINGBUT_ON;
        while (!hardwaretest_continue_button()) {
            adc_check.cur_val = potadc_buffer[adc_i];
            if (check_adc(&adc_check)) 
                break;
        }
        LED_INF1_ON;
		LED_INF2_ON;
        delay_ms(150);
		LED_INF1_OFF;
		LED_INF2_OFF;
        pause_until_button_released();
    }
}

void test_CV(void) {

}


