/*
 * AD9833.c
 *
 *  Created on: Aug 2, 2019
 *      Author: Quang Dan
 */

#include "AD9833.h"


//------------------------------------------------------------------------------------------------------------------------------------// AD9833 - Begin
///////////////////////////////////////////////////////////////////////////
void AD9833_SSI_Master_Configure()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
	SysCtlDelay(10);

	GPIOPinConfigure(GPIO_PD0_SSI3CLK);
	GPIOPinConfigure(GPIO_PD2_SSI3RX);
	GPIOPinConfigure(GPIO_PD3_SSI3TX);

	GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_0);

	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_1);

	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);

	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_3 | GPIO_PIN_0, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
	AD9833_deselect();
	SysCtlDelay(10);
	SSIConfigSetExpClk(SSI3_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_2, SSI_MODE_MASTER, 20000000, 8);
	SSIEnable(SSI3_BASE);

}
///////////////////////////////////////////////////////////////////////////
void AD9833_select()
{
	GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0);
}
///////////////////////////////////////////////////////////////////////////
void AD9833_deselect()
{
	GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_PIN_1);
}
///////////////////////////////////////////////////////////////////////////
void AD9833_Init()
{
	AD9833_select();
	WriteRegister_AD9833(0x2100);
	WriteRegister_AD9833(0x7728);// FR0, f = 29kHz, MSB
	WriteRegister_AD9833(0x4017);// FR0, f = 29kHz, LSB
	WriteRegister_AD9833(0xACEA);// FR1, f=  24kHz, MSB
	WriteRegister_AD9833(0x8013);// FR1, f=  24kHz, LSB
	WriteRegister_AD9833(0xC000);// Phase offset of FR0 = 0
	WriteRegister_AD9833(0xE000);// Phase offset of FR1 = 0
	WriteRegister_AD9833(0x2000);// Output is sine
	AD9833_deselect();
}
///////////////////////////////////////////////////////////////////////////
void WriteFrequencyAD9833(long frequency, uint16_t waveform)
{
	 uint16_t MSB, LSB;
	 uint16_t phase = 0;

	 uint32_t calculated_freq_word;
	 float AD9837Val = 0.00000000;

	 AD9837Val = (((float)(frequency))/25000000);
	 calculated_freq_word = AD9837Val*0x10000000;

	 MSB = (uint16_t)((calculated_freq_word & 0xFFFC000)>>14); //14 bits
	 LSB = (uint16_t)(calculated_freq_word & 0x3FFF);

	 //Set control bits DB15 ande DB14 to 0 and one, respectively, for frequency register 0
	 LSB |= 0x4000;
	 MSB |= 0x4000;
	 phase &= 0xC000;
	 //Set the frequency==========================
	 AD9833_select();
	 WriteRegister_AD9833(LSB); //lower 14 bits
	 WriteRegister_AD9833(MSB); //upper 14 bits
	 WriteRegister_AD9833(phase); //mid-low
	 WriteRegister_AD9833(waveform); //sin
	 AD9833_deselect();
}
///////////////////////////////////////////////////////////////////////////
void WriteRegister_AD9833(uint16_t data)
{
	SSIDataPut(SSI3_BASE, (data>>8)&0xFF);// Transfer high byte
	while(SSIBusy(SSI3_BASE)){}
	SSIDataPut(SSI3_BASE, (data&0xFF));// Transfer low byte
	while(SSIBusy(SSI3_BASE)){}
}
//------------------------------------------------------------------------------------------------------------------------------------// AD9833 - End
