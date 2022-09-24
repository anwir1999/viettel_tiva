/*
 * cellular.h
 *
 *  Created on: Nov 01, 2020
 *      Author: chungnt@epi-tech.com.vn
 *
 *      
*/
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef Cellular_H
#define Cellular_H


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
//#include "All_User_Lib.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */
#define DEFAULT_SYSTEM_MODE "NO SERVICE"

typedef enum
{
	MODEM_OFFLINE = 0,
	MODEM_ONLINE,
} Modem_Online_State;

typedef enum
{
	CELL_IDLE = 0,
	CELL_RECONNECT,
	CELL_GETBTSINFOR,
	CELL_POWERON,
	CELL_RESET,
	CELL_BUSY, // using MQTT, FTP, HTTP, TCP/IP,...
	CELL_ONLYBTSINFOR,
} Cellular_State_t;

typedef struct
{
    bool Flag_Cell_GetBTInfor_Done;
} Cell_Flag_t;

typedef enum
{
	CELL_NONE_MODE = 0,
	CELL_CMD_MODE = 1,
	CELL_DATA_MODE = 2,
} Cell_Mode_t;

typedef enum
{
	CELL_BUSY_TCPIP = 1,
	CELL_BUSY_MQTT,
} Cell_Busy_State_t;
	
typedef struct
{
	uint8_t initModuleDone;
	Cellular_State_t cellState;
	uint8_t step;
	uint16_t getSignalLevelTimeOut;
	uint8_t csq;
	uint8_t BTSInfor;
	char systemMode[15];
	char IMEI[20];
	uint8_t firstTimePowerOn;
	uint8_t waitCellReady;
	uint8_t cellReady;
	uint8_t cellNotReady;
	uint8_t cellRDY2Use;
	uint8_t moduleNotMounted;
	uint16_t timeOutConnection;
	uint8_t onlineState;
	uint8_t busyReason;
	int MCC;
	int MNC;
	int LAC;
	int cell_ID;
	int16_t RSRP;
	int16_t RSRQ;
	int RSSI;
	bool Cell_Timer_Init;
} Cell_Manager_t;
	
/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* Definition of public (external) data types go here */





/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/* Function prototypes for public (external) functions go here */
void Cellular_InitModule(void);
void Cellular_HardReset(void);
void Cellular_SwitchMode(Cell_Mode_t newMode);
void Cellular_SwitchState(Cellular_State_t newState);
void Cellular_Change2BusyState(uint8_t reason);
uint8_t Cellular_GetBusyReason(void);
void Cellular_ManagerTask(void);
uint8_t Cellular_CheckReady2Use(void);
uint8_t Cellular_CheckSignalInfo(void);
void Cellular_ClearSignalInfo(void);
void Cellular_GetSignalLevel(void);

void Cellular_Ready(void *ResponseBuffer);
void Cellular_GPRS_event_reporting(void *ResponseBuffer);

void ConfigureTimer_Cellular(uint16_t timer_tick_ms);
int filter_comma(char *respond_data, int begin, int end, char *output);
void CPSI_Decode(char* str, int* MCC, int* MNC, int* LAC, int* cell_ID, int* RSSI, int16_t* RSRSP, int16_t* RSRQ);

uint8_t Cellular_LogInfo(char *buf);
#endif
#ifdef __cplusplus
}
#endif
