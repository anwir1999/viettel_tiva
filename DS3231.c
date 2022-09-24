/*
 * DS3231.c
 *
 *  Created on: Apr 15, 2017
 *      Author: Quang Dan
 */




#include "DS3231.h"

DS3231_Time_t DS3231_Time;

unsigned char bcd_to_decimal(unsigned char d)
{
	return ((d & 0x0F)+((d & 0xF0)>>4)*10);
}

unsigned char decimal_to_bcd(unsigned char d)
{
	return (((d/10)<<4) & 0xF0) | ((d%10) & 0x0F);
}

void DS3231_Write(uint8_t address, uint8_t subAddress, uint8_t data)
{
	I2C_DS3231_Send(address, 2, subAddress, data);
}
void AT24C32_Write(uint8_t address, uint8_t subAddress, uint8_t data)
{
	I2CMasterSlaveAddrSet(I2C_DS3231__Master_Base, address, false);
	while(I2CMasterBusy(I2C_DS3231__Master_Base))  {}

	I2CMasterDataPut(I2C_DS3231__Master_Base, 0x00); //WRITE
	I2CMasterControl(I2C_DS3231__Master_Base, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C_DS3231__Master_Base))  {}

	I2CMasterDataPut(I2C_DS3231__Master_Base, subAddress); //ADDRESS
	I2CMasterControl(I2C_DS3231__Master_Base,  I2C_MASTER_CMD_BURST_SEND_CONT);
	while(I2CMasterBusy(I2C_DS3231__Master_Base))  {}

	I2CMasterDataPut(I2C_DS3231__Master_Base, data);    //DATA
	I2CMasterControl(I2C_DS3231__Master_Base, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(I2C_DS3231__Master_Base))  {}
}
uint8_t AT24C32_readByte(uint8_t slave_addr, uint8_t subAddress)
{
	I2CMasterSlaveAddrSet(I2C_DS3231__Master_Base, slave_addr, false);
	while(I2CMasterBusy(I2C_DS3231__Master_Base))  {}

	I2CMasterDataPut(I2C_DS3231__Master_Base, 0x00);    //READ
	I2CMasterControl(I2C_DS3231__Master_Base, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C_DS3231__Master_Base))  {}

	I2CMasterDataPut(I2C_DS3231__Master_Base, subAddress); //ADDRESS
	I2CMasterControl(I2C_DS3231__Master_Base, I2C_MASTER_CMD_BURST_SEND_CONT);
	while(I2CMasterBusy(I2C_DS3231__Master_Base))  {}



	I2CMasterSlaveAddrSet(I2C_DS3231__Master_Base, slave_addr, true);
	while(I2CMasterBusy(I2C_DS3231__Master_Base))  {}

	I2CMasterControl(I2C_DS3231__Master_Base, I2C_MASTER_CMD_SINGLE_RECEIVE );
	while(I2CMasterBusy(I2C_DS3231__Master_Base))  {}
	return I2CMasterDataGet(I2C_DS3231__Master_Base);                  // Return data read from slave register
}
void AT24C32_DoublePutData(double val, uint16_t first_address)
{
	uint8_t byte[4];
	float2Bytes(byte, val);
	uint8_t i;
	for(i = 0; i < 4; i++)
	{
		AT24C32_Write(AT24C32_Address, i + first_address, byte[i]);
		SysCtlDelay(SysCtlClockGet()/5);
	}
}
double AT24C32_DoubleReadData(uint16_t first_address)
{
	uint8_t byte[4];
	double val = 0;
	uint8_t i;
	for(i = 0; i < 4; i++)
	{
		byte[i] = AT24C32_readByte(AT24C32_Address, first_address + i);
		SysCtlDelay(SysCtlClockGet()/5);
	}
	val = bytesToFloatA(byte[3],byte[2],byte[1],byte[0]);
	return val;
}
void float2Bytes(uint8_t bytes_temp[4],float float_variable)
{
  memcpy(bytes_temp, (unsigned char*) (&float_variable), 4);
}
float bytesToFloatA(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
{
    float output;

    *((uint8_t*)(&output) + 3) = b0;
    *((uint8_t*)(&output) + 2) = b1;
    *((uint8_t*)(&output) + 1) = b2;
    *((uint8_t*)(&output) + 0) = b3;

    return output;
}
uint8_t DS3231_readByte(uint8_t slave_addr, uint8_t reg)
{
	I2CMasterSlaveAddrSet(I2C_DS3231__Master_Base, slave_addr, false);
	I2CMasterDataPut(I2C_DS3231__Master_Base, reg);
	I2CMasterControl(I2C_DS3231__Master_Base, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C_DS3231__Master_Base));
	I2CMasterSlaveAddrSet(I2C_DS3231__Master_Base, slave_addr, true);
	I2CMasterControl(I2C_DS3231__Master_Base, I2C_MASTER_CMD_SINGLE_RECEIVE);
	while(I2CMasterBusy(I2C_DS3231__Master_Base));
	return I2CMasterDataGet(I2C_DS3231__Master_Base);                  // Return data read from slave register
}
void I2C_DS3231_Send(uint8_t slave_addr, uint8_t num_of_args, ...)
{
    I2CMasterSlaveAddrSet(I2C_DS3231__Master_Base, slave_addr, false);
    va_list vargs;
    va_start(vargs, num_of_args);
    I2CMasterDataPut(I2C_DS3231__Master_Base, va_arg(vargs, uint32_t));
    if(num_of_args == 1)
    {
        I2CMasterControl(I2C_DS3231__Master_Base, I2C_MASTER_CMD_SINGLE_SEND);
        while(I2CMasterBusy(I2C_DS3231__Master_Base));
        va_end(vargs);
    }
    else
    {
        I2CMasterControl(I2C_DS3231__Master_Base, I2C_MASTER_CMD_BURST_SEND_START);
        while(I2CMasterBusy(I2C_DS3231__Master_Base));
        uint8_t i;
        for(i = 1; i < (num_of_args - 1); i++)
        {
            I2CMasterDataPut(I2C_DS3231__Master_Base, va_arg(vargs, uint32_t));
            I2CMasterControl(I2C_DS3231__Master_Base, I2C_MASTER_CMD_BURST_SEND_CONT);
            while(I2CMasterBusy(I2C_DS3231__Master_Base));
        }
        I2CMasterDataPut(I2C_DS3231__Master_Base, va_arg(vargs, uint32_t));
        I2CMasterControl(I2C_DS3231__Master_Base, I2C_MASTER_CMD_BURST_SEND_FINISH);
        while(I2CMasterBusy(I2C_DS3231__Master_Base));
        va_end(vargs);
    }
}
void setTime(unsigned char hSet, unsigned char mSet, unsigned char sSet, short am_pm_state, short hour_format)
{
	unsigned char tmp = 0;
	unsigned char change = decimal_to_bcd(sSet);
	DS3231_Write(DS3231_Address, secondREG, change);
	change = decimal_to_bcd(mSet);
	DS3231_Write(DS3231_Address, minuteREG, change);
	if(hour_format == 1)
	{
		if(am_pm_state == 1)
		{
			tmp = 0x60;
		}
		else
		{
			tmp = 0x40;
		}
		DS3231_Write(DS3231_Address, hourREG, tmp | (0x1F & (decimal_to_bcd(hSet))));

	}
	else
	{
		DS3231_Write(DS3231_Address, hourREG, ((tmp | (0x3F & (decimal_to_bcd(hSet))))));
	}
}
void setDate(uint8_t daySet, uint8_t dateSet, uint8_t monthSet, uint8_t yearSet)
{
	DS3231_Write(DS3231_Address, dayREG, decimal_to_bcd(daySet));
	DS3231_Write(DS3231_Address, dateREG, decimal_to_bcd(dateSet));
	DS3231_Write(DS3231_Address, monthREG, decimal_to_bcd(monthSet));
	DS3231_Write(DS3231_Address, yearREG, decimal_to_bcd(yearSet));
}
void DS3231_init()
{
	DS3231_I2C_Config();
	DS3231_Write(DS3231_Address, controlREG, 0x00);
	DS3231_Write(DS3231_Address, statusREG, 0x08);
}

void GetTime(uint8_t *p3, uint8_t *p2, uint8_t *p1, short *p0, short hour_format)
{
	unsigned char tmp = 0;
	*p1 =  DS3231_readByte(DS3231_Address, secondREG);
	*p1 = bcd_to_decimal(*p1);
	*p2 = DS3231_readByte(DS3231_Address, minuteREG);
	*p2 = bcd_to_decimal(*p2);
	if(hour_format == 1)
	{
		tmp = DS3231_readByte(DS3231_Address, hourREG);
		tmp &=  0x20;
		*p0 = (short) (tmp >> 5);
		*p3 = 0x1F & DS3231_readByte(DS3231_Address, hourREG);
		*p3 = bcd_to_decimal(*p3);
	}
	else
	{
		*p3 = 0x3F & DS3231_readByte(DS3231_Address, hourREG);
		*p3 = bcd_to_decimal(*p3);
	}
}
void GetDate(uint8_t *p4, uint8_t *p3, uint8_t *p2, uint8_t *p1)
{
         *p1 = DS3231_readByte(DS3231_Address, yearREG);
         *p1 = bcd_to_decimal(*p1);
         *p2 = (0x1F & DS3231_readByte(DS3231_Address, monthREG));
         *p2 = bcd_to_decimal(*p2);
         *p3 = (0x3F & DS3231_readByte(DS3231_Address, dateREG));
         *p3 = bcd_to_decimal(*p3);
         *p4 = (0x07 & DS3231_readByte(DS3231_Address, dayREG));
         *p4 = bcd_to_decimal(*p4);
}
void DS3231_Get_Time_Date(void)
{
    GetTime(&DS3231_Time.hr, &DS3231_Time.min, &DS3231_Time.s, am, _24_hour_format);
    GetDate(&DS3231_Time.dy, &DS3231_Time.dt, &DS3231_Time.mt, &DS3231_Time.yr);
}
void DS3231_I2C_Config(void)
{
	SysCtlPeripheralEnable(I2C_DS3231__Master_Sys_Periph);

	SysCtlPeripheralReset(I2C_DS3231__Master_Sys_Periph);

	SysCtlPeripheralEnable(I2C_DS3231__Master_GPIO);

	GPIOPinConfigure(I2C_DS3231__Master_SCL);
	GPIOPinConfigure(I2C_DS3231__Master_SDA);

	GPIOPinTypeI2CSCL(I2C_DS3231__Master_Port_Base, I2C_DS3231__Master_Pin_SCL);
	GPIOPinTypeI2C(I2C_DS3231__Master_Port_Base, I2C_DS3231__Master_Pin_SDA);

	I2CMasterInitExpClk(I2C_DS3231__Master_Base, SysCtlClockGet(), false); //400kbps.
	HWREG(I2C_DS3231__Master_Base + I2C_O_FIFOCTL) = 80008000;
}
