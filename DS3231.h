/*
 * DS3231.h
 *
 *  Created on: Apr 15, 2017
 *      Author: Quang Dan
 */

#ifndef DS3231_H_
#define DS3231_H_



#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include "math.h"
#include "driverlib/rom.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "driverlib/rom.h"
#include "driverlib/i2c.h"
#include "inc/hw_i2c.h"

#define AT24C32_Address 0x57

#define DS3231_Address 0x68
#define DS3231_Read_addr ((DS3231_Address << 1) | 0x01)
#define DS3231_Write_addr ((DS3231_Address << 1) & 0xFE)
#define secondREG 0x00
#define minuteREG 0x01
#define hourREG 0x02
#define dayREG 0x03
#define dateREG 0x04
#define monthREG 0x05
#define yearREG 0x06
#define alarm1secREG 0x07
#define alarm1minREG 0x08
#define alarm1hrREG 0x09
#define alarm1dateREG 0x0A
#define alarm2minREG 0x0B
#define alarm2hrREG 0x0C
#define alarm2dateREG 0x0D
#define controlREG 0x0E
#define statusREG 0x0F
#define ageoffsetREG 0x10
#define tempMSBREG 0x11
#define tempLSBREG 0x12
#define _24_hour_format 0
#define _12_hour_format 1
#define am 0
#define pm 1

#define I2C_DS3231__Master_Base   						I2C1_BASE
#define I2C_DS3231__Master_Sys_Periph 					SYSCTL_PERIPH_I2C1
#define I2C_DS3231__Master_GPIO							SYSCTL_PERIPH_GPIOA
#define I2C_DS3231__Master_Port_Base					GPIO_PORTA_BASE
#define I2C_DS3231__Master_SCL							GPIO_PA6_I2C1SCL
#define I2C_DS3231__Master_SDA							GPIO_PA7_I2C1SDA
#define I2C_DS3231__Master_Pin_SCL						GPIO_PIN_6
#define I2C_DS3231__Master_Pin_SDA						GPIO_PIN_7

typedef struct
{
    uint8_t s;
    uint8_t min;
    uint8_t hr;
    uint8_t dy;
    uint8_t dt;
    uint8_t mt;
    uint8_t yr;
    uint8_t old_s;
} DS3231_Time_t;


//setDate(7, 19, 5, 18);
//setTime(10, 05, 30, am, _24_hour_format);

unsigned char bcd_to_decimal(unsigned char d);
unsigned char decimal_to_bcd(unsigned char d);
void DS3231_Write(uint8_t address, uint8_t subAddress, uint8_t data);
void I2C_DS3231_Send(uint8_t slave_addr, uint8_t num_of_args, ...);
uint8_t DS3231_readByte(uint8_t slave_addr, uint8_t reg);
void GetTime(unsigned char *p3, unsigned char *p2, unsigned char *p1, short *p0, short hour_format);
void GetDate(uint8_t *p4, uint8_t *p3, uint8_t *p2, uint8_t *p1);
void DS3231_Get_Time_Date(void);
void DS3231_I2C_Config(void);
void DS3231_init();
void setDate(unsigned char daySet, unsigned char dateSet, unsigned char monthSet, unsigned char yearSet);
void setTime(unsigned char hSet, unsigned char mSet, unsigned char sSet, short am_pm_state, short hour_format);
void AT24C32_Write(uint8_t address, uint8_t subAddress, uint8_t data);
uint8_t AT24C32_readByte(uint8_t slave_addr, uint8_t subAddress);
void AT24C32_DoublePutData(double val, uint16_t first_address);
double AT24C32_DoubleReadData(uint16_t first_address);
void float2Bytes(uint8_t bytes_temp[4],float float_variable);
float bytesToFloatA(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);

#endif /* DS3231_H_ */
