/*
 * Common.h
 *
 *  Created on: Jan 19, 2022
 *      Author: QuangDan
 */

#ifndef COMMON_H_
#define COMMON_H_

// needed for driverlib rom stuff
#define TARGET_IS_BLIZZARD_RA1 1

#include "inc/hw_gpio.h" //for macro declaration of GPIO_O_LOCK and GPIO_O_CR
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
#include "driverlib/rom_map.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "driverlib/pwm.h"
#include "string.h"
#include "driverlib/ssi.h"
#include "uartstdio.h"

#define BUF_SIZE (1024)

#define NB_IoT                      0
#define GSM                         1
#define Both_NB_GSM                 2

char Network_Type_Str[10];
#define DAM_BUF_TX 2048
#define DAM_BUF_RX 1024

typedef enum
{
    PAIR = 0,
    UNPAIR,
    STARTUP,
    LOCATION,
    LOWBATTERY,
    FULLBATTERY,
    STARTMOTION,
    STOPMOTION,
    SOS,
    DCF_ACK,
    UNPAIR_GET,
    OFF_ACK,
    FOTA_SUCCESS,
    FOTA_FAIL,
} VTAG_MessageType;

typedef enum
{
    B_UNPAIR = 5,
    B_SHUTDOWN = 3,
    B_FOTA = 4,
    B_RESTART = 2,
} Button_Task;

typedef struct _CFG
{
    int Mode    ;
    int Period;
    char Type[5];
    int CC      ;
    int Network ;
    int Accuracy;
    char Server_Timestamp[15];
} CFG;

typedef struct
{
    int16_t RSRP;
    int16_t RSRQ;
    int16_t RSSI;
}Network_Signal;

typedef struct
{
    long Device_Timestamp;
    uint8_t Bat_Level;
    uint16_t Bat_Voltage;
}Device_Param;

typedef struct
{
    bool Flag_ScanNetwok;
    bool Flag_Cycle_Completed;
    bool Flag_ActiveNetwork;
    bool Flag_DeactiveNetwork;
    bool Flag_Network_Active;
    bool Flag_Network_Check_OK;
    bool Flag_Wait_Exit;
    bool Flag_GPS_Started;
    bool Flag_GPS_Stopped;
    bool Flag_Timer_GPS_Run;
    bool Flag_Device_Ready;
    bool Flag_WifiScan_Request;
    bool Flag_WifiScan_End;
    bool Flag_WifiCell_OK;
    bool Flag_Restart7070G_OK;
    bool Flag_CFUN_0_OK;
    bool Flag_CFUN_1_OK;
    bool Flag_Control_7070G_GPIO_OK;
    bool Flag_SelectNetwork_OK;
    bool Flag_NeedToProcess_GPS;
    bool Flag_MQTT_Stop_OK;
    bool Flag_RebootNetworkModule_OK;
    bool Flag_NeedToRebootNetworkModule;
    // Need adding to reset parameters funtion
    bool Flag_MQTT_Connected;
    bool Flag_MQTT_Sub_OK;
    bool Flag_MQTT_Publish_OK;
    bool Flag_MQTT_SubMessage_Processing;

    uint8_t Task_Selection;
}Device_Flag;

typedef enum
{
    FL_TASK_RFID = 1,
    FL_TASK_GNSS = 2,
    FL_TASK_CAM = 3,
} Device_Task;

typedef struct
{
    /* data */
    const char content[500];
} ATC_List_t;

typedef enum
{
    SIMCOM_EVEN_OK = 0,
    SIMCOM_EVEN_TIMEOUT,
    SIMCOM_EVEN_ERROR,
    SIMCOM_EVEN_NULL,
} SIMCOM_ResponseEvent_t;
typedef void (*SIMCOM_SendATCallBack_t)(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);

typedef enum
{
    QUECTEL_EVEN_OK = 0,
    QUECTEL_EVEN_TIMEOUT,
    QUECTEL_EVEN_ERROR,
} Quectel_ResponseEvent_t;
typedef void (*Quectel_SendATCallBack_t)(Quectel_ResponseEvent_t event, void *ResponseBuffer);

//-----------------------------------------------------------------------------------------------------// MQTT
// Device ID
//#define Device_ID_TW            "78639604-8276-480d-8962-65a54f3833d2"                                  // Master 3
#define Device_ID_TW            "48505448-5c50-4a47-aad3-7c7c8e6ec378"                                  // Master 4


#define MQTT_TX_Str_Buf_Lenght  500


typedef struct
{
    /* data */
    char dataRec[1024];
    char dataTrans[1024];
} MQTTdataType_t;
MQTTdataType_t MQTTdataType;
//--------------------------------------------------------------------------------------------// Define VTAG testing command
typedef struct
{
    bool Flag_Test_Start;
    bool Flag_Test_Stop;
    bool Flag_Test_Acc;
    bool Flag_Test_IMSI;
    bool Flag_Test_SNeSIM;
    bool Flag_Test_Id;
    bool Flag_Test_NBWFC;
    bool Flag_Test_NB;
    bool Flag_Test_WFC;
    bool Flag_Test_GPS2G;
    bool Flag_Test_GPS;
    bool Flag_Test_2G;
    bool Flag_Test_Charging;
    bool Flag_Test_Vol;
    bool Flag_Test_Sleep;
} VTAG_Testing_Flag;

typedef enum
{
    VTAG_TEST_START = 0,
    VTAG_TEST_STOP,
    VTAG_TEST_ACC,
    VTAG_TEST_IMSI,
    VTAG_TEST_SN,
    VTAG_TEST_ID,
    VTAG_TEST_NBWFC,
    VTAG_TEST_NB,
    VTAG_TEST_WFC,
    VTAG_TEST_GPS2G,
    VTAG_TEST_GPS,
    VTAG_TEST_2G,
    VTAG_TEST_CHARGE,
    VTAG_TEST_BATVOL,
    VTAG_TEST_SLEEP,
} TestCommand_Enum;



char Mqtt_TX_Str[MQTT_TX_Str_Buf_Lenght];
char MQTT_ID_Topic[75];

//--------------------------------------------------------------------------------------------------------------// Define for cell
int MCC, MNC, LAC, cell_ID, RSSI;
//--------------------------------------------------------------------------------------------------------------// Define for simcom response event
SIMCOM_ResponseEvent_t AT_RX_event;
//--------------------------------------------------------------------------------------------------------------// Define for Flag

#endif /* COMMON_H_ */
