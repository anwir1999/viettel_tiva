/*
 * ADS1241.h
 *
 *  Created on: Apr 27, 2018
 *      Author: Quang Dan
 */

#ifndef ADS1241_H_
#define ADS1241_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/adc.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "driverlib/pwm.h"

#define ADS1242_PORT_BASE    					GPIO_PORTC_BASE
#define ADS1242_SYSCTL_PERIPH					SYSCTL_PERIPH_GPIOC
// P3.7
#define ADS1242_DSYNC   0x80
// P3.6
#define ADS1242_PDWN    0x40
// P3.3
#define ADS1242_SCLK    GPIO_PIN_4
// P3.5
#define ADS1242_CS_2    GPIO_PIN_3// PORTF
#define ADS1242_CS_1    GPIO_PIN_7// PORTC

// GPIO2 (P0) on the port expander
#define CLK_KICK_START_EN   0x01
// GPIO3 (P1) on the port expander
#define ADS1242_BUFEN   0x02

#define ADS1242_DIN     GPIO_PIN_6//GPIO_PIN_6

#define ADS1242_DOUT    GPIO_PIN_5//GPIO_PIN_5

#define ADS1242_POL     0x04


#define ADS1242_DRDY    GPIO_PIN_2//PF2

#define ADS1242_DRDY_1_PORT_BASE						GPIO_PORTE_BASE
#define ADS1242_DRDY_1  								GPIO_PIN_5//PE5
#define ADS1242_DRDY_1_SYSCTL_PERIPH					SYSCTL_PERIPH_GPIOE
#define ADS1242_DRDY_1_INT_GPIO  						INT_GPIOE

#define ADS1242_DRDY_2_PORT_BASE						GPIO_PORTF_BASE
#define ADS1242_DRDY_2  								GPIO_PIN_2//PF2
#define ADS1242_DRDY_2_SYSCTL_PERIPH					SYSCTL_PERIPH_GPIOF
#define ADS1242_DRDY_2_INT_GPIO  						INT_GPIOF

#define ADS1242_RESET   0x08

#define ADS1242_NO_ERROR            0
#define ADS1242_TIMEOUT_WARNING  -10

// define commands (see page 18 in the data sheet
#define ADS1242_CMD_RDATA    0x01
#define ADS1242_CMD_RDATAC   0x03
#define ADS1242_CMD_STOPC    0x0f
#define ADS1242_CMD_RREG     0x10
#define ADS1242_CMD_WREG     0x50
#define ADS1242_CMD_SELFCAL  0xf0
#define ADS1242_CMD_SELFOCAL 0xf1
#define ADS1242_CMD_SELFGCAL 0xf2
#define ADS1242_CMD_SYSOCAL  0xf3
#define ADS1242_CMD_SYSGCAL  0xf4
#define ADS1242_CMD_WAKEUP   0xfb
#define ADS1242_CMD_DSYNC    0xfc
#define ADS1242_CMD_SLEEP    0xfd
#define ADS1242_CMD_RESET    0xfe

// define the ADS1242 register values
#define ADS1242_SETUP_REGISTER   0x00
#define ADS1242_MUX_REGISTER     0x01
#define ADS1242_ACR_REGISTER     0x02
#define ADS1242_ODAC_REGISTER    0x03
#define ADS1242_DIO_REGISTER     0x04
#define ADS1242_DIR_REGISTER     0x05
#define ADS1242_IOCON_REGISTER   0x06
#define ADS1242_OCR0_REGISTER    0x07
#define ADS1242_OCR1_REGISTER    0x08
#define ADS1242_OCR2_REGISTER    0x09
#define ADS1242_FSR0_REGISTER    0x0a
#define ADS1242_FSR1_REGISTER    0x0b
#define ADS1242_FSR2_REGISTER    0x0c
#define ADS1242_DOR2_REGISTER    0x0d
#define ADS1242_DOR1_REGISTER    0x0e
#define ADS1242_DOR0_REGISTER    0x0f

// define multiplexer codes
#define ADS1242_MUXP_AIN0   0x00
#define ADS1242_MUXP_AIN1   0x10
#define ADS1242_MUXP_AIN2   0x20
#define ADS1242_MUXP_AIN3   0x30
#define ADS1242_MUXP_AIN4   0x40
#define ADS1242_MUXP_AIN5   0x50
#define ADS1242_MUXP_AIN6   0x60
#define ADS1242_MUXP_AIN7   0x70
#define ADS1242_MUXP_AINCOM 0x80

#define ADS1242_MUXN_AIN0   0x00
#define ADS1242_MUXN_AIN1   0x01
#define ADS1242_MUXN_AIN2   0x02
#define ADS1242_MUXN_AIN3   0x03
#define ADS1242_MUXN_AIN4   0x04
#define ADS1242_MUXN_AIN5   0x05
#define ADS1242_MUXN_AIN6   0x06
#define ADS1242_MUXN_AIN7   0x07
#define ADS1242_MUXN_AINCOM 0x08

// define gain codes
#define ADS1242_GAIN_1      0x00
#define ADS1242_GAIN_2      0x01
#define ADS1242_GAIN_4      0x02
#define ADS1242_GAIN_8      0x03
#define ADS1242_GAIN_16     0x04
#define ADS1242_GAIN_32     0x05
#define ADS1242_GAIN_64     0x06
#define ADS1242_GAIN_128    0x07

// define ACR register bits
#define N_DRDY_BIT          0x80
#define U_N_B_BIT           0x40
#define SPEED_BIT           0x20
#define BUFEN_BIT           0x10
#define BIT_ORDER_BIT       0x08
#define RANGE_BIT           0x04
#define DR_1_BIT            0x02
#define DR_0_BIT            0x01

// low level functions
int ADS1242Configure(void);
void ADS1242Init(void);
int ADS1242WaitForDataReady(int Timeout);
void ADS1242AssertCS( int fAssert, int CS);
void ADS1242SendByte(int );
unsigned char ADS1242ReceiveByte();

// higher level functions
long ADS1242ReadData(int fWaitForDataReady, int CS);
int ADS1242ReadRegister(int StartAddress, int NumRegs, unsigned * pData, int CS);
int ADS1242WriteRegister(int StartAddress, int NumRegs, unsigned * pData, int CS);
int ADS1242SendResetCommand(int CS);
int ADS1242SendStopCommand(int CS);
int ADS1242SendRestartCommand(int CS);

//
int ADS1242SetChannel(int MuxCode, int CS);
int ADS1242SetGain(int GainCode, int CS);



#endif /* ADS1241_H_ */
