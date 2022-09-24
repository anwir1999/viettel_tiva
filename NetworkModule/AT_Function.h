/*
 * AT_Function.h
 *
 *  Created on: Jan 19, 2022
 *      Author: QuangDan
 */

#ifndef NETWORKMODULE_AT_FUNCTION_H_
#define NETWORKMODULE_AT_FUNCTION_H_
#include "Common.h"
#include "../third_party/fatfs/src/ff.h"
#include "../third_party/fatfs/src/diskio.h"

#define TIMER_ATC_PERIOD 100

//-----------------------------------------------------------------------------------------//
// Simcom AT functions
//-----------------------------------------------------------------------------------------//
void Simcom_SendATCommand();
void Simcom_RetrySendATC();
void Simcom_ATC_SendATCommand(const char *Command, char *ExpectResponse, uint32_t Timeout, uint8_t RetryCount, SIMCOM_SendATCallBack_t CallBackFunction);
void Simcom_ATResponse_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
typedef struct
{
    char CMD[DAM_BUF_TX];
    uint32_t lenCMD;
    char ExpectResponseFromATC[20];
    uint32_t TimeoutATC;
    uint32_t CurrentTimeoutATC;
    uint8_t RetryCountATC;
    SIMCOM_SendATCallBack_t SendATCallBack;
    bool Flag_RX_End;
    char AT_RX_Data[BUF_SIZE];
    uint32_t AT_RX_ArrayIndex;
}ATCommand_t;
ATCommand_t SIMCOM_ATCommand;
//-----------------------------------------------------------------------------------------//
// Quectel AT functions
//-----------------------------------------------------------------------------------------//
void Quectel_SendATCommand();
void Quectel_ATC_SendATCommand(const char *Command, char *ExpectResponse, uint32_t Timeout, uint8_t RetryCount, Quectel_SendATCallBack_t CallBackFunction);
void Quectel_RetrySendATC();
void Quectel_ATResponse_Callback(Quectel_ResponseEvent_t event, void *ResponseBuffer);
void Quectel_SendATCommand_Hex(uint8_t special_char);
void Quectel_SendATCommand_Str(const char *str);
typedef struct
{
    char CMD[DAM_BUF_TX];
    uint32_t lenCMD;
    char ExpectResponseFromATC[20];
    uint32_t TimeoutATC;
    uint32_t CurrentTimeoutATC;
    uint8_t RetryCountATC;
    Quectel_SendATCallBack_t SendATCallBack;
}Quectel_ATCommand_t;
Quectel_ATCommand_t Quectel_ATCommand;

//-----------------------------------------------------------------------------------------//
// Common functions
//-----------------------------------------------------------------------------------------//
void WaitandExitLoop(bool *Flag);
void UART0_Send(char *cmd);
void UART2_Send(char *cmd);
void Configure_AT_Command(void);
void ConfigureTimer_A0(uint16_t timer_tick_ms);
void ConfigureUART2_GNSS(void);
void ATCommand_RX_Process();

#endif /* NETWORKMODULE_AT_FUNCTION_H_ */
