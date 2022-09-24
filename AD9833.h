/*
 * AD9833.h
 *
 *  Created on: Aug 2, 2019
 *      Author: Quang Dan
 */

#ifndef AD9833_H_
#define AD9833_H_

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/adc.h"
#include "driverlib/pwm.h"


#define Sine 				0x2000
#define Square				0x2020
#define Triagle 			0x2002

// Khai báo nguyên mẫu hàm
void AD9833_SSI_Master_Configure();
void AD9833_Init();
void WriteFrequencyAD9833(long frequency, uint16_t waveform);
void WriteRegister_AD9833(uint16_t data);
void AD9833_select();
void AD9833_deselect();


#endif /* AD9833_H_ */
