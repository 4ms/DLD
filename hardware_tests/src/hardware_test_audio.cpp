#include "hardware_test_audio.h"
#include "hardware_test_util.h"
#include "CodecCallbacks.h"
extern "C" {
#include "dig_pins.h"
#include "leds.h"
#include "globals.h"
#include "codec_CS4271.h"
#include "i2s.h"
}

static void setup_outs_as_LFOs(void);

//FlagStatus codeca_timeout=0, codeca_ackfail=0, codeca_buserr=0;
//FlagStatus codecb_timeout=0, codecb_ackfail=0, codecb_buserr=0;

////Todo: enable these to help diagnose
//void I2C1_ER_IRQHandler(void)
//{
//	codeca_timeout = I2C_GetFlagStatus(CODECA_I2C, I2C_FLAG_TIMEOUT);
//	codeca_ackfail = I2C_GetFlagStatus(CODECA_I2C, I2C_FLAG_AF);
//	codeca_buserr = I2C_GetFlagStatus(CODECA_I2C, I2C_FLAG_BERR);
//}

//void I2C2_ER_IRQHandler(void)
//{
//	codecb_timeout = I2C_GetFlagStatus(CODECB_I2C, I2C_FLAG_TIMEOUT);
//	codecb_ackfail = I2C_GetFlagStatus(CODECB_I2C, I2C_FLAG_AF);
//	codecb_buserr = I2C_GetFlagStatus(CODECB_I2C, I2C_FLAG_BERR);
//}


void test_codec_init(void) {
	all_leds_off();
	pause_until_button_released();

	Codec_GPIO_Init();

	LED_LOOP2_ON;
	Codec_B_AudioInterface_Init(SAMPLERATE);
	LED_LOOP2_OFF;

	LED_LOOP1_ON;
	Codec_A_AudioInterface_Init(SAMPLERATE);
	LED_LOOP1_OFF;

	LED_LOOP1_ON;
	LED_LOOP2_ON;
	init_audio_dma();
	LED_LOOP1_OFF;
	LED_LOOP2_OFF;

	uint32_t err;
	LED_REV2_ON;
	err = Codec_B_Register_Setup(DCINPUT_JUMPER);
	if (err) {
		const uint32_t bailout_time=50; //5 seconds
		uint32_t ctr = bailout_time;
		while (ctr) {
			LED_LOOP2_ON;
			delay_ms(50);
			LED_LOOP2_OFF;
			delay_ms(50);
			if (hardwaretest_continue_button())
				ctr--;
			else 
				ctr = bailout_time;
		}
	}
	LED_REV2_OFF;

	LED_REV1_ON;
	err = Codec_A_Register_Setup(DCINPUT_JUMPER);
	if (err) {
		while (1) {
		const uint32_t bailout_time=50; //5 seconds
		uint32_t ctr = bailout_time;
		while (ctr) {
			LED_LOOP1_ON;
			delay_ms(50);
			LED_LOOP1_OFF;
			delay_ms(50);
			if (hardwaretest_continue_button())
				ctr--;
			else 
				ctr = bailout_time;
		}
		}
	}
	LED_REV1_OFF;
}

static SkewedTriOsc testWaves[4];

//Test Audio Outs
//"Outer" button lights turn on (Reverse A/B)
//Each output jack outputs a left-leaning triangle, rising in freq as you go left->right
//Out A should have two tones mixed together, then patching a dummy cable into Out B should mute the higher tone
//
void test_audio_out(void) {
	LED_REV1_ON;
	LED_REV2_ON;

	float max = ((1<<15)-1);
	float min = -max - 1.0f;

	testWaves[0].init(100, 0.2, max, min, 0, SAMPLERATE);
	testWaves[1].init(150, 0.2, max, min, 0, SAMPLERATE);
	testWaves[2].init(225, 0.2, max, min, 0, SAMPLERATE);
	testWaves[3].init(175, 0.2, max, min, 0, SAMPLERATE);

	CodecCallbacks_TwoCodecs::leftOutCodec1 = &testWaves[0];
	CodecCallbacks_TwoCodecs::rightOutCodec1 = &testWaves[1];
	CodecCallbacks_TwoCodecs::leftOutCodec2 = &testWaves[2];
	CodecCallbacks_TwoCodecs::rightOutCodec2 = &testWaves[3];
	set_codec_callback(CodecCallbacks_TwoCodecs::testWavesOut);

	Start_I2SDMA_Channel1();
	Start_I2SDMA_Channel2();

	uint8_t continue_armed=0;
	while (1) {
		if (hardwaretest_continue_button()) {LED_PINGBUT_ON; continue_armed = 1;}
		else if (!hardwaretest_continue_button() && continue_armed) break;
		delay_ms(80);
	}

	LED_PINGBUT_OFF;
	LED_REV1_OFF;
	LED_REV2_OFF;
}

//Test Audio Ins
//"Inner" button lights turn on (Hold A/B)
//Each output jack outputs the same tones as in the last test, but at 1/2 the volume
//Patch a cable from any output jack to an input jack, and the two tones will mix
//Note: patching Out B to In A will produce a feedback loop unless a dummy cable is patched into In B
//Recommended to just avoid patching Out B->In A.
//Note: You can hear three tones by patching Send A or B-> In B while listening to Out A

void test_audio_in(void) {
	LED_INF1_ON;
	LED_INF2_ON;

	float max = ((1<<15)-1) / 2.0f;
	float min = -max - 1.0f;

	testWaves[0].init(200, 0.2, max, min, 0, SAMPLERATE);
	testWaves[1].init(300, 0.2, max, min, 0, SAMPLERATE);
	testWaves[2].init(450, 0.2, max, min, 0, SAMPLERATE);
	testWaves[3].init(350, 0.2, max, min, 0, SAMPLERATE);

	set_codec_callback(CodecCallbacks_TwoCodecs::passthruPlusTestWave);

	pause_until_button_pressed();
	delay_ms(80);

	LED_PINGBUT_OFF;
	LED_INF1_OFF;
	LED_INF2_OFF;
}

