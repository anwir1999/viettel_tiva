/*
 * AT_Function.c
 *
 *  Created on: Jan 19, 2022
 *      Author: QuangDan
 */

#ifndef NETWORKMODULE_AT_FUNCTION_C_
#define NETWORKMODULE_AT_FUNCTION_C_

#include "AT_Function.h"
#include "GNSS.h"
#include "../MQTT/MQTT_HwLayer.h"
#include "../DS3231.h"
#include "../SD/SD.h"
#include "../RFID/RFID.h"
#include "../TFT_LCD_Lib/IFTSPI2_2LCD.h"
#include "../CAM/CAM.h"

extern char GNSS_Buf[GNSS_Buf_Size];
extern GNSS_Manager_t GNSS_Manager;
extern DS3231_Time_t DS3231_Time;
extern SD_FileName_t SD_FileName;
extern CAM_Param_t CAM_Param;
//extern Mqtt_Manager_t Mqtt_Manager;
//extern RFID_Manager_t RFID_Manager;
extern uint8_t MQTT_MsgType;

Device_Flag D_Flag_t;
uint16_t Network_Scan_Counter;
SIMCOM_ResponseEvent_t AT_RX_event;

//#define TIMER_ATC_PERIOD      100
//-----------------------------------------------------------------------------------------//
// Simcom AT functions
//-----------------------------------------------------------------------------------------//
void Simcom_SendATCommand()
{
    UARTprintf("Send: %s", SIMCOM_ATCommand.CMD);
    UART2_Send((char *) SIMCOM_ATCommand.CMD);
}
//-----------------------------------------------------------------------------------------//
void Simcom_ATC_SendATCommand(const char *Command, char *ExpectResponse, uint32_t Timeout, uint8_t RetryCount, SIMCOM_SendATCallBack_t CallBackFunction)
{
    //UARTprintf("Send: %s\r", Command);
    strcpy(SIMCOM_ATCommand.CMD, Command);
    //UARTprintf("Send: %s\r", SIMCOM_ATCommand.CMD);
    SIMCOM_ATCommand.lenCMD = strlen(SIMCOM_ATCommand.CMD);
    strcpy(SIMCOM_ATCommand.ExpectResponseFromATC, ExpectResponse);
    SIMCOM_ATCommand.RetryCountATC = RetryCount;
    SIMCOM_ATCommand.SendATCallBack = CallBackFunction;
    SIMCOM_ATCommand.TimeoutATC = Timeout;
    SIMCOM_ATCommand.CurrentTimeoutATC = 0;
    AT_RX_event = SIMCOM_EVEN_NULL;

    Simcom_SendATCommand();
}
//-----------------------------------------------------------------------------------------//
void Simcom_RetrySendATC()
{
    Simcom_SendATCommand();
}
//-----------------------------------------------------------------------------------------//
void Simcom_ATResponse_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
    AT_RX_event = event;
    if(event == SIMCOM_EVEN_OK)
    {
        UARTprintf("SIMCOM_EVENT_OK\r\n");
        D_Flag_t.Flag_Wait_Exit = true;
        D_Flag_t.Flag_Device_Ready = true;
    }
    else if(event == SIMCOM_EVEN_TIMEOUT)
    {
        UARTprintf("SIMCOM_EVENT_TIMEOUT\r\n");
        D_Flag_t.Flag_Wait_Exit = true;
    }
    else if(event == SIMCOM_EVEN_ERROR)
    {
        UARTprintf("SIMCOM_EVENT_ERROR\r\n");
        D_Flag_t.Flag_Wait_Exit = true;
    }
}
//-----------------------------------------------------------------------------------------//
// Quectel AT functions
//-----------------------------------------------------------------------------------------//
void Quectel_SendATCommand()
{
    UART2_Send((char *) SIMCOM_ATCommand.CMD);
    printf("Send: %s", Quectel_ATCommand.CMD);
    //uart_write_bytes(UART_NUM_0, (const char *) Quectel_ATCommand.CMD, sizeof(Quectel_ATCommand.CMD));
    //ESP_LOGI(TAG,"Send: %s", Quectel_ATCommand.CMD);
}
void Quectel_SendATCommand_Hex(uint8_t special_char)
{
    //uart_write_bytes(BC660K_UART_PORT_NUM, special_char, 1);
}
void Quectel_SendATCommand_Str(const char *str)
{
    //uart_write_bytes(BC660K_UART_PORT_NUM, (const char *) str, sizeof(str));
    printf("Send: %s", str);
}
//-----------------------------------------------------------------------------------------//
void Quectel_ATC_SendATCommand(const char *Command, char *ExpectResponse, uint32_t Timeout, uint8_t RetryCount, Quectel_SendATCallBack_t CallBackFunction)
{
    strcpy(Quectel_ATCommand.CMD, Command);
    Quectel_ATCommand.lenCMD = strlen(Quectel_ATCommand.CMD);
    strcpy(Quectel_ATCommand.ExpectResponseFromATC, ExpectResponse);
    Quectel_ATCommand.RetryCountATC = RetryCount;
    Quectel_ATCommand.SendATCallBack = CallBackFunction;
    Quectel_ATCommand.TimeoutATC = Timeout;
    Quectel_ATCommand.CurrentTimeoutATC = 0;

    Quectel_SendATCommand();
}
//-----------------------------------------------------------------------------------------//
void Quectel_RetrySendATC()
{
    Quectel_SendATCommand();
}
//-----------------------------------------------------------------------------------------//
void Quectel_ATResponse_Callback(Quectel_ResponseEvent_t event, void *ResponseBuffer)
{
    //AT_RX_event = event;
    if(event == QUECTEL_EVEN_OK)
    {
        //ESP_LOGW(TAG, "QUECTEL_EVENT_OK\r\n");
        D_Flag_t.Flag_Wait_Exit = true;
        D_Flag_t.Flag_Device_Ready = true;
    }
    else if(event == QUECTEL_EVEN_TIMEOUT)
    {
        //ESP_LOGE(TAG, "QUECTEL_EVENT_TIMEOUT\r\n");
        D_Flag_t.Flag_Wait_Exit = true;
    }
    else if(event == QUECTEL_EVEN_ERROR)
    {
        //ESP_LOGE(TAG, "QUECTEL_EVENT_ERROR\r\n");
        D_Flag_t.Flag_Wait_Exit = true;
    }
}
//-----------------------------------------------------------------------------------------//
// Common functions
//-----------------------------------------------------------------------------------------//
void UART2_Send(char *cmd)
{
    while(UARTBusy(UART2_BASE));
    UARTCharPut(UART2_BASE, *cmd++);
    while(*cmd != '\0')
    {
        UARTCharPut(UART2_BASE, *cmd++);
    }
    UARTCharPut(UART2_BASE, '\r'); //CR
    UARTCharPut(UART2_BASE, '\n'); //LF
}
void UART0_Send(char *cmd)
{
    while(ROM_UARTBusy(UART0_BASE));
    ROM_UARTCharPutNonBlocking(UART0_BASE, *cmd++);
    while(*cmd != '\0')
    {
        ROM_UARTCharPutNonBlocking(UART0_BASE, *cmd++);
    }
}
//-----------------------------------------------------------------------------------------//
void WaitandExitLoop(bool *Flag)
{
    while(1)
    {
        if(*Flag == true)
        {
            *Flag = false;
            break;
        }
        SysCtlDelay(SysCtlClockGet()/10);
        //vTaskDelay(50 / RTOS_TICK_PERIOD_MS);
    }
}
//-----------------------------------------------------------------------------------------// Check timeout
void Timer0IntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    if(SIMCOM_ATCommand.TimeoutATC > 0 && SIMCOM_ATCommand.CurrentTimeoutATC < SIMCOM_ATCommand.TimeoutATC)
    {
       SIMCOM_ATCommand.CurrentTimeoutATC += TIMER_ATC_PERIOD;
       //UARTprintf("AT timeout count: %d\r\n", SIMCOM_ATCommand.CurrentTimeoutATC);
       if(SIMCOM_ATCommand.CurrentTimeoutATC >= SIMCOM_ATCommand.TimeoutATC)
       {
           //UARTprintf("Timeout \r\n");
           SIMCOM_ATCommand.CurrentTimeoutATC -= SIMCOM_ATCommand.TimeoutATC;
           if(SIMCOM_ATCommand.RetryCountATC > 0)
           {
               SIMCOM_ATCommand.RetryCountATC--;
               Simcom_RetrySendATC();
           }
           else
           {
               if(SIMCOM_ATCommand.SendATCallBack != NULL)
               {
                   SIMCOM_ATCommand.TimeoutATC = 0;
                   SIMCOM_ATCommand.SendATCallBack(SIMCOM_EVEN_TIMEOUT, "@@@");
               }
           }
       }
    }
}
//-----------------------------------------------------------------------------------------// UART RX for AT command
void Configure_AT_Command(void)
{
    memset(SIMCOM_ATCommand.AT_RX_Data, 0, sizeof(SIMCOM_ATCommand.AT_RX_Data));

    ConfigureTimer_A0(1000/TIMER_ATC_PERIOD);
    ConfigureUART2_GNSS();

    // Create a CSV file to store the data of location and speed
    GNSS_Manager.flag_GNSS_file_created = false;

    DS3231_Get_Time_Date();
    UARTprintf("[DS3231] Time: %02d/%02d/20%02d %02d:%02d:%02d\r\n",DS3231_Time.dt, DS3231_Time.mt, DS3231_Time.yr, DS3231_Time.hr, DS3231_Time.min, DS3231_Time.s);
    sprintf(SD_FileName.SD_GNSS_FileName, "Location/GNSS_%02d_%02d_20%02d_%02d_%02d_%02d.csv", DS3231_Time.dt,  DS3231_Time.mt,  DS3231_Time.yr,  DS3231_Time.hr,  DS3231_Time.min,  DS3231_Time.s);
    //SD_Init(SD_FileName.SD_GNSS_FileName);
    SD_CreateFile(SD_FileName.SD_GNSS_FileName);
    SD_Open(SD_FileName.SD_GNSS_FileName);
    SD_GNSS_Write("");
    SD_Close();
}
void ConfigureTimer_A0(uint16_t timer_tick_ms)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, (SysCtlClockGet() / timer_tick_ms) -1);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();
    TimerEnable(TIMER0_BASE, TIMER_A);
}
//-----------------------------------------------------------------------------------------//
void GPIOPinUnlockGPIO(uint32_t ui32Port, uint8_t ui8Pins) {
    HWREG(ui32Port + GPIO_O_LOCK) = GPIO_LOCK_KEY;      // Unlock the port
    HWREG(ui32Port + GPIO_O_CR) |= ui8Pins;             // Unlock the Pin
}
void ConfigureUART2_GNSS(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
    //Give it time for clocks to start
    SysCtlDelay(10);
    GPIOPinUnlockGPIO(GPIO_PORTD_BASE, GPIO_PIN_7);
    GPIOPinConfigure(GPIO_PD6_U2RX);
    GPIOPinConfigure(GPIO_PD7_U2TX);
    GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    UARTConfigSetExpClk(UART2_BASE, SysCtlClockGet(), 921600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    IntEnable(INT_UART2);
    UARTIntEnable(UART2_BASE, UART_INT_RX | UART_INT_RT);
    IntMasterEnable();

    ROM_IntPrioritySet(INT_UART2, 0x00);
    //UARTEnable(UART2_BASE);
}
//-----------------------------------------------------------------------------------------//
void ATCommand_RX_Process()
{
    // Return event OK
    if(SIMCOM_ATCommand.ExpectResponseFromATC[0] != 0 && strstr((const char*)SIMCOM_ATCommand.AT_RX_Data, SIMCOM_ATCommand.ExpectResponseFromATC))
    {
       SIMCOM_ATCommand.ExpectResponseFromATC[0] = 0;
       if(SIMCOM_ATCommand.SendATCallBack != NULL)
       {
           SIMCOM_ATCommand.TimeoutATC = 0;
           SIMCOM_ATCommand.SendATCallBack(SIMCOM_EVEN_OK, SIMCOM_ATCommand.AT_RX_Data);
       }
    }

    // Check error
    if(strstr((const char*)SIMCOM_ATCommand.AT_RX_Data, "ERROR"))
    {
       if(SIMCOM_ATCommand.SendATCallBack != NULL)
       {
           SIMCOM_ATCommand.SendATCallBack(SIMCOM_EVEN_ERROR, SIMCOM_ATCommand.AT_RX_Data);
       }
    }

    // Copy GNSS data to buffer
    if(strstr(SIMCOM_ATCommand.AT_RX_Data, "+CGNSSINFO:"))
    {
        memset(GNSS_Buf, 0, sizeof(GNSS_Buf));
        strcpy(GNSS_Buf, (const char*)SIMCOM_ATCommand.AT_RX_Data);
    }


    // MQTT lost
    if(strstr((const char*)SIMCOM_ATCommand.AT_RX_Data, "+CMQTTCONNLOST:"))
    {
        MQTT_RetryConnect();
    }

    // MQTT ACK from server
    if(strstr((const char*)SIMCOM_ATCommand.AT_RX_Data, "\"code\":0"))
    {
        if(MQTT_MsgType == FL_MQTT_RFID)
        {
            GPIOPinWrite(LCD_LED_BASE, LCD_LED_PIN, 0);
        }
        MQTT_SubACK_Callback();
    }

    // MQTT sub message from server
    if(strstr((const char*)SIMCOM_ATCommand.AT_RX_Data, "\"cam\":1"))
    {
        CAM_Param.Flag_CAM_Require = true;
        //CAM_TakePhoto();
    }

    // Reset if the device is offline
    if(strstr((const char*)SIMCOM_ATCommand.AT_RX_Data, "+CPSI: NO SERVICE,Offline"))
    {
        SysCtlReset();
    }

    // Reset AT command buffer
    memset(SIMCOM_ATCommand.AT_RX_Data, 0, sizeof(SIMCOM_ATCommand.AT_RX_Data));
    SIMCOM_ATCommand.AT_RX_ArrayIndex = 0;
}

void UART2IntHandler(void)
{
    uint32_t ui32Status;
    ui32Status = UARTIntStatus(UART2_BASE, true);
    UARTIntClear(UART2_BASE, ui32Status);
    while(ROM_UARTCharsAvail(UART2_BASE))
    {
        uint16_t RX_char = ROM_UARTCharGetNonBlocking(UART2_BASE);
        ROM_UARTCharPutNonBlocking(UART0_BASE, RX_char);
        if(RX_char != '\r' && RX_char != '\n' )
        {
            SIMCOM_ATCommand.AT_RX_Data[SIMCOM_ATCommand.AT_RX_ArrayIndex] = RX_char;
            SIMCOM_ATCommand.AT_RX_ArrayIndex++;
        }
        if(RX_char == '\n')
        {
            SIMCOM_ATCommand.AT_RX_Data[SIMCOM_ATCommand.AT_RX_ArrayIndex] = '\0';
            if(SIMCOM_ATCommand.AT_RX_ArrayIndex > 0)
            {
                SIMCOM_ATCommand.Flag_RX_End = true;
                ATCommand_RX_Process();
            }
        }

    }
}



#endif /* NETWORKMODULE_AT_FUNCTION_C_ */
