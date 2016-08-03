/*
 * ITM.h
 *
 *  Created on: Jun 17, 2016
 *      Author: design
 */

#ifndef ITM_H_
#define ITM_H_


void ITM_Init(uint32_t SWOSpeed);
void ITM_SendValue (int port, uint32_t value);
void ITM_Print(int port, const char *p);
void ITM_Disable(void);




#endif /* ITM_H_ */
