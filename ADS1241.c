/*
 * ADS1241.c
 *
 *  Created on: Apr 27, 2018
 *      Author: Quang Dan
 */

#include "ADS1241.h"


int tosc = 5;
///////////////////////////////////////////////////////////////////////////
int ADS1242Configure(void)
{
	SysCtlPeripheralEnable(ADS1242_SYSCTL_PERIPH);

   GPIOPinTypeGPIOOutput(ADS1242_PORT_BASE, ADS1242_SCLK | ADS1242_DIN| ADS1242_CS_1);
   GPIOPinWrite(ADS1242_PORT_BASE, ADS1242_SCLK  | ADS1242_DIN, 0);
   GPIOPinTypeGPIOInput(ADS1242_PORT_BASE, ADS1242_DOUT);
   GPIOPinWrite(ADS1242_PORT_BASE, ADS1242_CS_1, ADS1242_CS_1);

   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
   GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, ADS1242_CS_2);
   GPIOPinWrite(GPIO_PORTF_BASE, ADS1242_CS_2, ADS1242_CS_2);

   SysCtlPeripheralEnable(ADS1242_DRDY_2_SYSCTL_PERIPH);
   GPIOPinTypeGPIOInput(ADS1242_DRDY_2_PORT_BASE, ADS1242_DRDY_2);
   GPIOIntTypeSet(ADS1242_DRDY_2_PORT_BASE, ADS1242_DRDY_2, GPIO_FALLING_EDGE);
   GPIOIntEnable(ADS1242_DRDY_2_PORT_BASE,ADS1242_DRDY_2 );
   IntEnable(ADS1242_DRDY_2_INT_GPIO);


   SysCtlPeripheralEnable(ADS1242_DRDY_1_SYSCTL_PERIPH);
   GPIOPinTypeGPIOInput(ADS1242_DRDY_1_PORT_BASE, ADS1242_DRDY_1);
   GPIOIntTypeSet(ADS1242_DRDY_1_PORT_BASE, ADS1242_DRDY_1, GPIO_FALLING_EDGE);
   GPIOIntEnable(ADS1242_DRDY_1_PORT_BASE,ADS1242_DRDY_1 );
   IntEnable(ADS1242_DRDY_1_INT_GPIO);


   IntMasterEnable();
   return ADS1242_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////
void ADS1242Init(void)
{
	uint8_t i;
	unsigned ACRVal;
	int chan = 0;
	chan = ADS1242_MUXP_AIN3 | ADS1242_MUXN_AIN2;
	// Thiết lập cho ADC2
	ADS1242AssertCS(0, 1);// Vô hiệu hóa ADC1 bằng chân CS
	ADS1242SendResetCommand(2);
	ADS1242SetGain(ADS1242_GAIN_1, 2);
	ADS1242SetChannel(chan, 2);
	//data rate = 15Hz (4.91MHz, SPEED = 1)
	ACRVal = 0 ; //SPEED = 0

	ADS1242WriteRegister(ADS1242_ACR_REGISTER, 1, &ACRVal, 2);
	ADS1242AssertCS(1, 2);
	ADS1242SendByte(ADS1242_CMD_SELFCAL);
	ADS1242AssertCS(0, 2);
	for (i=0; i<4; i++);
	ADS1242WaitForDataReady(0);
	SysCtlDelay(SysCtlClockGet()/20);

	//Thiết lập cho ADC1
	ADS1242AssertCS(0, 2);// Vô hiệu hóa ADC2 bằng chân CS
	ADS1242SendResetCommand(1);
	ADS1242SetGain(ADS1242_GAIN_1, 1);
	ADS1242SetChannel(chan, 1);
	//data rate = 15Hz (4.91MHz, SPEED = 1)
	ACRVal = 0 ; //SPEED = 0

	ADS1242WriteRegister(ADS1242_ACR_REGISTER, 1, &ACRVal, 1);
	ADS1242AssertCS(1, 1);
	ADS1242SendByte(ADS1242_CMD_SELFCAL);
	ADS1242AssertCS(0, 1);

	for (i=0; i<4; i++);
	ADS1242WaitForDataReady(0);
}

///////////////////////////////////////////////////////////////////////////
int ADS1242WaitForDataReady(int Timeout)
{
   if (Timeout > 0)
   {
      // wait for /DRDY = 1

      while (!GPIOPinRead(GPIO_PORTF_BASE, ADS1242_DRDY) && (Timeout-- >= 0))
         ;
      // wait for /DRDY = 0
      while ( GPIOPinRead(GPIO_PORTF_BASE, ADS1242_DRDY) && (Timeout-- >= 0))
         ;
      if (Timeout < 0)
         return ADS1242_TIMEOUT_WARNING;
   }
   else
   {
      // wait for /DRDY = 1
      while (!GPIOPinRead(GPIO_PORTF_BASE, ADS1242_DRDY))
         ;
      // wait for /DRDY = 0
      while ( GPIOPinRead(GPIO_PORTF_BASE, ADS1242_DRDY))
         ;
   }
   return ADS1242_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////
void ADS1242AssertCS( int fAssert, int CS)
{
   if (fAssert)
   {
	   if(CS == 1) 			GPIOPinWrite(ADS1242_PORT_BASE, ADS1242_CS_1, 0);
	   else if(CS == 2) 	GPIOPinWrite(GPIO_PORTF_BASE, ADS1242_CS_2, 0);
   }
   else
   {
	   if(CS == 1) 			GPIOPinWrite(ADS1242_PORT_BASE, ADS1242_CS_1, ADS1242_CS_1);
	   else if(CS == 2) 	GPIOPinWrite(GPIO_PORTF_BASE, ADS1242_CS_2, ADS1242_CS_2);
   }
}
///////////////////////////////////////////////////////////////////////////
void ADS1242SendByte(int Byte)
{
	int i;
	for (i=0; i<8; i++)
	{
	  if (Byte & 0x80)
		  GPIOPinWrite(ADS1242_PORT_BASE, ADS1242_DIN, ADS1242_DIN);
	  else
		  GPIOPinWrite(ADS1242_PORT_BASE, ADS1242_DIN, 0);

	  GPIOPinWrite(ADS1242_PORT_BASE, ADS1242_SCLK, ADS1242_SCLK);
	  SysCtlDelay(1*tosc);
	  GPIOPinWrite(ADS1242_PORT_BASE, ADS1242_SCLK, 0);
	  SysCtlDelay(1*tosc);
	  Byte <<= 1;
	}
	GPIOPinWrite(ADS1242_PORT_BASE, ADS1242_DIN, 0);
}
///////////////////////////////////////////////////////////////////////////
unsigned char ADS1242ReceiveByte(void)
{

   unsigned char Result = 0;
   int i;
   for (i=0; i<8; i++)
   {
      Result <<= 1;
      GPIOPinWrite(ADS1242_PORT_BASE, ADS1242_SCLK, ADS1242_SCLK);
      SysCtlDelay(1*tosc);
      if(GPIOPinRead(ADS1242_PORT_BASE, ADS1242_DOUT)) Result |=  1;
      GPIOPinWrite(ADS1242_PORT_BASE, ADS1242_SCLK, 0);
      SysCtlDelay(1*tosc);
   }
   return Result;
}
///////////////////////////////////////////////////////////////////////////
long ADS1242ReadData(int fWaitForDataReady, int CS)
{
   long Data;
   // if requested, synchronize to /DRDY
   if (fWaitForDataReady)
      ADS1242WaitForDataReady(0);

   // assert CS to start transfer
   ADS1242AssertCS(1, CS);

   // send the command byte
   ADS1242SendByte(ADS1242_CMD_RDATA);

   // delay according to the data sheet
   SysCtlDelay(50 * tosc);

   // get the conversion result
   Data = ADS1242ReceiveByte();
   Data = (Data << 8) | ADS1242ReceiveByte();
   Data = (Data << 8) | ADS1242ReceiveByte();

   // sign extend data
   if (Data & 0x800000)
      Data |= 0xff000000;

   // de-assert CS
   ADS1242AssertCS(0, CS);
   return Data;
}
///////////////////////////////////////////////////////////////////////////
int ADS1242ReadRegister(int StartAddress, int NumRegs, unsigned * pData, int CS)
{
   int i;

   // assert CS to start transfer
   ADS1242AssertCS(1, CS);

   // send the command byte
   ADS1242SendByte(ADS1242_CMD_RREG | (StartAddress & 0x0f));

   // send the command argument
   ADS1242SendByte(NumRegs-1);

   // ROM_SysCtlDelay according to the data sheet
   SysCtlDelay(50 * tosc);

   // get the register content
   for (i=0; i< NumRegs; i++)
   {
      *pData++ = ADS1242ReceiveByte();
   }

   // de-assert CS
   ADS1242AssertCS(0, CS);
   return ADS1242_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////
int ADS1242WriteRegister(int StartAddress, int NumRegs, unsigned * pData, int CS)
{
   int i;

   // assert CS to start transfer
   ADS1242AssertCS(1, CS);

   // send the command byte
   ADS1242SendByte(ADS1242_CMD_WREG | (StartAddress & 0x0f));

   // send the command argument
   ADS1242SendByte(NumRegs-1);

   // ROM_SysCtlDelay according to the data sheet
   SysCtlDelay(50 * tosc);

   // send the data bytes
   for (i=0; i< NumRegs; i++)
   {
      ADS1242SendByte(*pData++);
   }

   // de-assert CS
   ADS1242AssertCS(0, CS);
   return ADS1242_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////
int ADS1242SendResetCommand(int CS)
{
   // assert CS to start transfer
   ADS1242AssertCS(1, CS);

   // send the command byte
   ADS1242SendByte(ADS1242_CMD_RESET);

   // de-assert CS
   ADS1242AssertCS(0, CS);

   // wait for t11 (16 tosc)
   SysCtlDelay(16 * tosc);

   return ADS1242_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////
int ADS1242SendStopCommand(int CS)
{
   // assert CS to start transfer
   ADS1242AssertCS(1, CS);

   // send the command byte
   ADS1242SendByte(ADS1242_CMD_STOPC);

   // de-assert CS
   ADS1242AssertCS(0, CS);

   // wait for t11 (16 tosc)
   SysCtlDelay(16 * tosc);

   return ADS1242_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////
int ADS1242SendRestartCommand(int CS)
{
   // assert CS to start transfer
   ADS1242AssertCS(1, CS);

   // send the command byte
   ADS1242SendByte(ADS1242_CMD_RDATAC);

   // de-assert CS
   ADS1242AssertCS(0, CS);

   // wait for t11 (16 tosc)
   SysCtlDelay(16 * tosc);

   return ADS1242_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////
int ADS1242SetChannel(int MuxCode, int CS)
{
   unsigned Temp =0;
   Temp |= MuxCode;
   ADS1242WriteRegister(ADS1242_MUX_REGISTER, 0x01, &Temp, CS);
   return ADS1242_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////
int ADS1242SetGain(int GainCode, int CS)
{
   unsigned Temp;

   // the gain code is only part of the register, so we have to read it back
   // and massage the new gain code into it
   ADS1242ReadRegister(ADS1242_SETUP_REGISTER, 0x01, &Temp, CS);

   // clear prev gain code;
   Temp &= ~0x07;
   Temp |= GainCode & 0x07;

   // write the register value containing the new gain code back to the ADS
   ADS1242WriteRegister(ADS1242_SETUP_REGISTER, 0x01, &Temp, CS);
   return ADS1242_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////



