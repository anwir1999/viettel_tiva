/*
 * RFID.c
 *
 *  Created on: Feb 22, 2022
 *      Author: QuangDan
 */
#include "RFID.h"
#include "../DS3231.h"
#include "../SD/SD.h"
#include "../MQTT/MQTT_HwLayer.h"

RFID_Data_t RFID_Data;
RFID_Manager_t RFID_Manager;

extern uint8_t MQTT_MsgType;
extern DS3231_Time_t DS3231_Time;
extern SD_FileName_t SD_FileName;
extern SD_Param_t   SD_Param;
extern Device_Flag D_Flag_t;
//------------------------------------------------------------------------------------------------------------------------------------//
void RFID_Task_Execute(bool state)
{
    if(state == true)
    {
        UARTprintf("[RFID] Request RFID\r\n");
        UART6_Send("FL,RFID,1\r\n");
        UARTEnable(UART6_BASE);
        MQTT_MsgType = FL_MQTT_RFID;
        RFID_Manager.Flag_RFID_Data_Waiting = true;
        D_Flag_t.Task_Selection = FL_TASK_RFID;
    }
    else
    {
        UARTDisable(UART6_BASE);
    }
}
void Configure_RFID(void)
{
    RFID_Manager.RFID_Data_Waiting_Counter = 0;
    ConfigureUART6_RFID();
    DS3231_Get_Time_Date();
    UARTprintf("[DS3231] Time: %02d/%02d/20%02d %02d:%02d:%02d\r\n",DS3231_Time.dt, DS3231_Time.mt, DS3231_Time.yr, DS3231_Time.hr, DS3231_Time.min, DS3231_Time.s);
    sprintf(SD_FileName.SD_RFID_FileName, "RFID/RFID_%02d_%02d_20%02d_%02d_%02d_%02d.txt", DS3231_Time.dt,  DS3231_Time.mt,  DS3231_Time.yr,  DS3231_Time.hr,  DS3231_Time.min,  DS3231_Time.s);

    SD_CreateFile(SD_FileName.SD_RFID_FileName);
    SD_Open(SD_FileName.SD_RFID_FileName);
    SD_GNSS_Write("");
    SD_Close();
}
//------------------------------------------------------------------------------------------------------------------------------------//
void ConfigureUART6_RFID(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);
    GPIOPinConfigure(GPIO_PD4_U6RX);
    GPIOPinConfigure(GPIO_PD5_U6TX);
    GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    UARTClockSourceSet(UART6_BASE, UART_CLOCK_PIOSC);
    UARTConfigSetExpClk(UART6_BASE, 16000000, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    //UARTConfigSetExpClk(UART6_BASE, SysCtlClockGet(), 921600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTEnable(UART6_BASE);
    IntEnable(INT_UART6);
    UARTIntEnable(UART6_BASE, UART_INT_RX | UART_INT_RT);
    IntMasterEnable();
}
//------------------------------------------------------------------------------------------------------------------------------------//
void UART6_Send(char *cmd)
{
    while(ROM_UARTBusy(UART6_BASE));
    ROM_UARTCharPutNonBlocking(UART6_BASE, *cmd++);
    while(*cmd != '\0')
    {
        ROM_UARTCharPutNonBlocking(UART6_BASE, *cmd++);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------//
void RFID_ReceiveData(void)
{
   if(ROM_UARTCharsAvail(UART6_BASE))
   {
        // Receive and send data
        uint16_t RX_char = ROM_UARTCharGetNonBlocking(UART6_BASE);
        ROM_UARTCharPutNonBlocking(UART0_BASE, RX_char);
        if(RX_char == '{')
        {
            RFID_Manager.Flag_RFID_RX_Begin = true;
            RFID_Manager.RFID_RX_Counter = 0;
        }
        if(RFID_Manager.Flag_RFID_RX_Begin == true)
        {
            RFID_Data.RFID_Data_Str[RFID_Manager.RFID_RX_Counter++] = RX_char;
            if(RFID_Manager.RFID_RX_Counter >= RFID_BUF_LENGTH)
            {
                RFID_Manager.RFID_RX_Counter = 0;
            }
            if(RX_char == '}')
            {
                RFID_Data.RFID_Data_Str[RFID_Manager.RFID_RX_Counter++] = '\r';
                RFID_Data.RFID_Data_Str[RFID_Manager.RFID_RX_Counter++] = '\n';
                RFID_Data.RFID_Data_Str[RFID_Manager.RFID_RX_Counter] = '\0';
                RFID_Manager.RFID_RX_Counter = 0;
                RFID_Manager.Flag_RFID_Data_Ready = true;
                RFID_Manager.Flag_RFID_RX_End = true;
                RFID_Manager.Flag_RFID_RX_Begin = false;
            }
        }
   }
}
//------------------------------------------------------------------------------------------------------------------------------------//
void UART6IntHandler(void)
{
    uint32_t ui32Status;
    ui32Status = UARTIntStatus(UART6_BASE, true);
    UARTIntClear(UART6_BASE, ui32Status);
    RFID_ReceiveData();
}
