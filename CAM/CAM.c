/*
 * RFID.c
 *
 *  Created on: Feb 14, 2022
 *      Author: QuangDan
 */
#include "CAM.h"
#include "../SD/SD.h"
#include "../NetworkModule/AT_Function.h"
#include "../DS3231.h"
#include "../MQTT/MQTT_HwLayer.h"
#include "../NetworkModule/GNSS.h"
#include "../RFID/RFID.h"

CAM_Param_t CAM_Param;
extern bool Flag_SD_Done;
extern Device_Flag D_Flag_t;
extern uint8_t MQTT_MsgType;
extern DS3231_Time_t DS3231_Time;
extern char SD_FilePath[50];

char CRC_Str[10];
char CAM_Data_Str[1500];//2000

uint8_t CAM_RX_CRC_Idx = 0;
uint32_t CAM_RX_Data_Idx = 0;
uint16_t CAM_RX_DataBlock = 0;

extern SD_Param_t SD_Param;
extern SD_FileName_t SD_FileName;
// CAM
extern CAM_Param_t CAM_Param;
// GNSS
extern GNSS_Params_t  GNSS_Params;
extern GNSS_Manager_t GNSS_Manager;
// MQTT
extern uint8_t MQTT_MsgType;
extern Mqtt_Manager_t Mqtt_Manager;
// RFID
extern RFID_Manager_t RFID_Manager;
extern RFID_Data_t RFID_Data;


//-----------------------------------------------------------------------------------------//
void CAM_TakePhoto(void)
{
    if(Mqtt_Manager.mesIsBeingSent != PUBLISH_BUSY && Mqtt_Manager.flag_mqtt_ack_wait != true && Mqtt_Manager.connectStatus != MQTT_STT_DISCONNECTTING && GNSS_Manager.flag_GNSS_sending_inprocess == false &&  RFID_Manager.Flag_RFID_Sending_Inprocess == false)// Wait for publish and subscribe ack done
    {
        CAM_Param.Flag_CAM_Require = false;
        D_Flag_t.Task_Selection = FL_TASK_CAM;
        MQTT_MsgType = FL_MQTT_IMG;

        GNSS_EnableReading(false);

        IntDisable(INT_TIMER4A);
        IntDisable(INT_TIMER1A);

        DS3231_Get_Time_Date();
        UARTprintf("[DS3231] Time: %02d/%02d/20%02d %02d:%02d:%02d\r\n",DS3231_Time.dt, DS3231_Time.mt, DS3231_Time.yr, DS3231_Time.hr, DS3231_Time.min, DS3231_Time.s);
        sprintf(SD_FilePath, "Image/IMG_%02d_%02d_20%02d_%02d_%02d_%02d.txt", DS3231_Time.dt,  DS3231_Time.mt,  DS3231_Time.yr,  DS3231_Time.hr,  DS3231_Time.min,  DS3231_Time.s);

        //SD_Init();
        SD_CreateFile(SD_FilePath);

        // Disable AT UART
        UARTDisable(UART2_BASE);
        // Start taking photo
        UART7_Send("FL,CAM,1\r\n");
    }
}
//-----------------------------------------------------------------------------------------//
void ConfigureUART7_CAM(void)
{
    CAM_Param.Flag_CAM_Require = false;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);
    //Give it time for clocks to start
    SysCtlDelay(10);
    GPIOPinConfigure(GPIO_PE0_U7RX);
    GPIOPinConfigure(GPIO_PE1_U7TX);
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART7_BASE, SysCtlClockGet(), 1842000, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    //UARTFIFOEnable(UART7_BASE);

    IntEnable(INT_UART7);
    UARTIntEnable(UART7_BASE, UART_INT_RX | UART_INT_RT);
    IntMasterEnable();
}
//-----------------------------------------------------------------------------------------//
void UART7_Send(char *cmd)
{
    while(ROM_UARTBusy(UART7_BASE));
    ROM_UARTCharPutNonBlocking(UART7_BASE, *cmd++);
    while(*cmd != '\0')
    {
        ROM_UARTCharPutNonBlocking(UART7_BASE, *cmd++);
    }
    //UARTCharPut(UART7_BASE, '\r'); //CR
    //UARTCharPut(UART7_BASE, '\n'); //LF
}
//-----------------------------------------------------------------------------------------//
void CAM_ReceiveData(void)
{
   if(ROM_UARTCharsAvail(UART7_BASE))
   {
        // Receive and send data
        uint16_t RX_char = ROM_UARTCharGetNonBlocking(UART7_BASE);
        ROM_UARTCharPutNonBlocking(UART0_BASE, RX_char);

        // Begin character processing
        if(RX_char == '<')
        {
            CAM_RX_CRC_Idx = 0;
            CAM_RX_Data_Idx = 0;
            CAM_RX_DataBlock = 0;
        }

        // CRC processing
        if(RX_char == '*')// Begin of block of data
        {
            CAM_RX_DataBlock++;
            if(CAM_RX_DataBlock == 1)
            {
                CAM_Param.Flag_CAM_RX_Begin = true;
                CAM_RX_CRC_Idx = 0;
                CAM_Param.CAM_Data_Str[CAM_RX_Data_Idx++] = '<';
            }
            else
            {
                CAM_Param.Flag_CAM_RX_Begin = true;
                CAM_RX_CRC_Idx = 0;
                CAM_RX_Data_Idx = 0;
            }
        }
        else if(CAM_Param.Flag_CAM_RX_Begin == true && CAM_Param.Flag_CAM_RX_Data == false)
        {
            // End of CRC string
           if(RX_char == ',')
           {
               CAM_Param.Flag_CAM_RX_Data = true;
               CAM_Param.Flag_CAM_RX_Begin = false;
               CRC_Str[CAM_RX_CRC_Idx++] = '\0';
               CAM_RX_CRC_Idx = 0;

           }
           // CRC string
           else if(RX_char != ',' && RX_char != 'C' && RX_char != 'R' && RX_char != '=')
           {
               CRC_Str[CAM_RX_CRC_Idx++] = RX_char;
           }
        }

        // Data processing
        if(CAM_Param.Flag_CAM_RX_Data == true)
        {
           // Data
           if(RX_char != '#' && RX_char != ',')
           {
               CAM_Param.CAM_Data_Str[CAM_RX_Data_Idx] = RX_char;
               CAM_RX_Data_Idx++;
           }
           // End character of block of data
           else if(RX_char == '#')
           {
               CAM_Param.CAM_Data_Str[CAM_RX_Data_Idx] = '\0';
               CAM_RX_Data_Idx = 0;
               CAM_Param.Flag_CAM_RX_Begin = false;
               CAM_Param.Flag_CAM_RX_Data = false;
               CAM_Param.Flag_CAM_Data_Ready = true;
           }
        }

        // End character
        if(RX_char == '>')
        {
            CAM_Param.Flag_CAM_RX_End = true;
            CAM_RX_DataBlock = 0;
        }
   }
}
void UART7IntHandler(void)
{
    ROM_SysTickDisable();
    uint32_t ui32Status;
    ui32Status = UARTIntStatus(UART7_BASE, true);
    UARTIntClear(UART7_BASE, ui32Status);
    CAM_ReceiveData();
}

