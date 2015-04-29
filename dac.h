/*
 * dac.h
 *
 *  Created on: Jul 28, 2014
 *      Author: design
 */

#ifndef DAC_H_
#define DAC_H_

void Audio_DAC_Init(void);

uint32_t adjust_playback(int16_t offset);
uint32_t set_playback(uint8_t chan, int16_t playbackrate);

#endif /* DAC_H_ */
