/*
 * cellular.c
 *
 *  Created on: Nov 01, 2020
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "../NetworkModule/Common.h"
#include "../NetworkModule/AT_Function.h"
#include "../MQTT/MQTT_HwLayer.h"
#include "cellular.h"

#define USE_MQTT                1

extern Device_Flag D_Flag_t;
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

/* Global variables definitions go here */
Cell_Manager_t Cell_Manager;
Cell_Mode_t Cell_Mode;
Cell_Flag_t Cell_Flag;
/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* Definition of private datatypes go here */

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */

/* Function prototypes for private (static) functions go here */
static uint8_t CheckCellOK(void);
static uint8_t CheckCellIdle(void);
static void Cell_PowerOn_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void Cell_Ready_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void Cell_Get_BTS_Info_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */

/* Functions definitions go here, organised into sections */
/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
void Cellular_InitModule(void)
{
	//memcpy(Cell_Manager.systemMode, DEFAULT_SYSTEM_MODE, sizeof(DEFAULT_SYSTEM_MODE));
	memset(Cell_Manager.systemMode, 0, sizeof(Cell_Manager.systemMode));
	//Cell_Manager.cellState = CELL_IDLE;
	Cell_Manager.getSignalLevelTimeOut = 0;
	Cell_Manager.step = 0;
	Cell_Manager.firstTimePowerOn = 0;
	Cell_Manager.waitCellReady = 0;
	Cell_Manager.cellReady = 0;
	Cell_Manager.cellNotReady = 0;
	Cell_Manager.moduleNotMounted = 0;
	Cell_Manager.initModuleDone = 1;
	Cell_Manager.onlineState = MODEM_OFFLINE;
	
	Cell_Mode = CELL_CMD_MODE;

	//Cellular_SwitchState(CELL_GETBTSINFOR);
	Cellular_Ready("");
	//ConfigureTimer_Cellular(100);
}

/*********************************************************************************************************//**
 * @brief: Hard reset module by pull low/hig powerkey
 * @param
 * @retval  
 ************************************************************************************************************/
void Cellular_HardReset(void)
{
	
}

/*********************************************************************************************************//**
 * @brief: Switch to new mode
 * @param
 * @retval  
 ************************************************************************************************************/
void Cellular_SwitchMode(Cell_Mode_t newMode)
{
	Cell_Mode = newMode;
	
	switch(Cell_Mode)
	{
		case CELL_NONE_MODE:
			UARTprintf("[CELL] Mode: CELL_NONE_MODE\n");
			break;
		case CELL_CMD_MODE:
		    UARTprintf("[CELL] Mode: CELL_CMD_MODE\n");
			break;
		case CELL_DATA_MODE:
		    UARTprintf("[CELL] Mode: CELL_DATA_MODE\n");
			break;
	}
}

/*********************************************************************************************************//**
 * @brief: Switch to new state
 * @param
 * @retval  
 ************************************************************************************************************/
void Cellular_SwitchState(Cellular_State_t newState)
{
//	Cell_Manager.cellOldState = Cell_Manager.cellState;
	Cell_Manager.cellState = newState;
	switch(Cell_Manager.cellState)
	{
		case CELL_IDLE:
			Cell_Manager.busyReason = 0;
			UARTprintf("[CELL] Switch: CELL_IDLE\n");
			break;
		case CELL_GETBTSINFOR:
		    UARTprintf("[CELL] Switch: CELL_GETBTSINFOR\n");
			break;
		case CELL_POWERON:
		    UARTprintf("[CELL] Switch: CELL_POWERON\n");
			break;
		case  CELL_RESET:
		    UARTprintf("[CELL] Switch: CELL_RESET\n");
			break;
		case  CELL_RECONNECT:
		    UARTprintf("[CELL] Switch: CELL_RECONNECT\n");
			break;
		case CELL_BUSY:
		    UARTprintf("[CELL] Switch: CELL_BUSY [%d]\n", Cell_Manager.busyReason);
			break;
	}
	Cell_Manager.step = 0;
}

/*********************************************************************************************************//**
 * @brief: Switch to busy state
 * @param
 * @retval  
 ************************************************************************************************************/
void Cellular_Change2BusyState(uint8_t reason)
{
	Cell_Manager.busyReason = reason;
	Cellular_SwitchState(CELL_BUSY);
}

/*********************************************************************************************************//**
 * @brief: Get busy state
 * @param
 * @retval  
 ************************************************************************************************************/
uint8_t Cellular_GetBusyReason(void)
{
	return Cell_Manager.busyReason;
}
/*********************************************************************************************************//**
 * @brief: Handle state machine every 1s
 * @param
 * @retval  
 ************************************************************************************************************/
void Cellular_ManagerTask(void)
{
	if(Cell_Manager.initModuleDone == 0) return;
	if(Cell_Manager.moduleNotMounted == 1) return;
	
	//UARTprintf("[CELL] Cellular Manager Task\r\n");
	//UARTprintf("[CELL] Cellular_ManagerTask\r\n");
	if(Cell_Manager.firstTimePowerOn == 0)
	{
		if(CheckCellOK())
		{
			if(Cell_Manager.waitCellReady == 0) UARTprintf("[CELL] Wait for ready in 3s\r\n");
			UARTprintf("[CELL] Cell_Manager.waitCellReady %d\r\n", Cell_Manager.waitCellReady);
			if(++Cell_Manager.waitCellReady >= 3) 
			{
				Cell_Manager.waitCellReady = 0;
				Cell_Manager.firstTimePowerOn = 1;
				Cellular_SwitchState(CELL_POWERON);
			}
		}
		else 
		{
		    UARTprintf("[CELL] Initiating Cellular...\r\n");
			if(++Cell_Manager.cellNotReady >= 30)
			{
				Cell_Manager.moduleNotMounted = 1;
			}
		}
	}
	
	switch(Cell_Manager.cellState)
	{
		case CELL_POWERON:
			//if(Cell_Manager.step == 0)
			{
				Cell_Manager.step = 1;
				//Simcom_ATC_SendATCommand("AT+SWITCHSIM?\r\n", "OK", 1000, 3, Cell_PowerOn_Callback);

				Simcom_ATC_SendATCommand("AT+CGSN\r\n", "OK", 1000, 3, Cell_PowerOn_Callback);
			}
			break;
			
		case CELL_GETBTSINFOR:
		    //if(Cell_Manager.step == 0)
		    {
                #if(USE_TCP_IP == 1)
                if(Cell_Mode == CELL_CMD_MODE)
                {
                  Cell_Manager.step = 1;
                  Simcom_ATC_SendATCommand("AT+CSQ\r\n", "+CSQ:", "OK", 1000, 3, Cell_Get_BTS_Info_Callback);
                }
                else if(Cell_Mode == CELL_DATA_MODE)
                  Simcom_ATC_SendATCommand("+++", "OK", NULL, 3000, 5, Cell_Get_BTS_Info_Callback);
                #endif
                #if(USE_MQTT == 1)
                Cell_Manager.step = 1;
                //Simcom_ATC_SendATCommand("AT+CSQ\r\n", "+CSQ:", 1000, 3, Cell_Get_BTS_Info_Callback);
                Simcom_ATC_SendATCommand("AT+CPSI?\r\n", "+CPSI: LTE,Online", 1000, 20, Cell_Get_BTS_Info_Callback);
                #endif
                break;
		    }
		case CELL_ONLYBTSINFOR:
            {
                #if(USE_TCP_IP == 1)
                if(Cell_Mode == CELL_CMD_MODE)
                {
                  Cell_Manager.step = 1;
                  Simcom_ATC_SendATCommand("AT+CSQ\r\n", "+CSQ:", "OK", 1000, 3, Cell_Get_BTS_Info_Callback);
                }
                else if(Cell_Mode == CELL_DATA_MODE)
                  Simcom_ATC_SendATCommand("+++", "OK", NULL, 3000, 5, Cell_Get_BTS_Info_Callback);
                #endif
                #if(USE_MQTT == 1)
                Cell_Manager.step = 1;
                Simcom_ATC_SendATCommand("AT+CPSI?\r\n", "+CPSI: LTE,Online", 1000, 20, Cell_Get_BTS_Info_Callback);
                #endif
                break;
            }
		case CELL_IDLE:
			break;
		
		case CELL_RESET:
			break;
		
		case CELL_RECONNECT:
			break;
		
		case CELL_BUSY:
			break;
	}
}

/*********************************************************************************************************//**
 * @brief  : Check ready state
 * @param
 * @retval  
 ************************************************************************************************************/
uint8_t Cellular_CheckReady2Use(void)
{
	return Cell_Manager.cellRDY2Use;
}

/*********************************************************************************************************//**
 * @brief  : Check CSQ and System mode are valid or not
 * @param
 * @retval  
 ************************************************************************************************************/
uint8_t Cellular_CheckSignalInfo(void)
{
	return Cell_Manager.BTSInfor;
}

/*********************************************************************************************************//**
 * @brief  : Clear Signal Info
 * @param
 * @retval  
 ************************************************************************************************************/
void Cellular_ClearSignalInfo(void)
{
	Cell_Manager.BTSInfor = 0;
}
/*********************************************************************************************************//**
 * @brief  : Get CSQ and System mode
 * @param
 * @retval  
 ************************************************************************************************************/
void Cellular_GetSignalLevel(void)
{
	#if(USE_TCP_IP == 1)
	if(Cell_Mode == CELL_CMD_MODE)
	#endif
		Simcom_ATC_SendATCommand("[CELL] AT+CSQ\r\n", "OK", 1000, 3, Cell_Get_BTS_Info_Callback);
}

/*********************************************************************************************************//**
 * @brief  : +CPIN:
 * @param
 * @retval  
 ************************************************************************************************************/
void Cellular_Ready(void *ResponseBuffer)
{
	UARTprintf("[CELL] Ready to use AT command\r\n");
	Simcom_ATC_SendATCommand("ATE0\r\n", "OK", 3000, 5, Cell_Ready_Callback);
}

/*********************************************************************************************************//**
 * @brief  : +CGEV:
 * @param
 * @retval  
 ************************************************************************************************************/
void Cellular_GPRS_event_reporting(void *ResponseBuffer)
{
	if(strstr((const char*)ResponseBuffer, "ME DETACH"))
	{
		UARTprintf("[CELL] Not Ready\r\n");
		Cell_Manager.cellReady = 0;
		Cell_Manager.cellRDY2Use = 0;
	}
}

/*********************************************************************************************************//**
 * @brief  : CELL_LOG
 * @param	uint8_t initModuleDone;
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
 * @retval  
 ************************************************************************************************************/
uint8_t Cellular_LogInfo(char *buf)
{
	if(strstr(buf, "CELL_LOG"))
	{
		UARTprintf("[CELL_LOG] %d:%d:%s:%s:%d:%d\r\n", Cell_Manager.cellState, Cell_Manager.csq, Cell_Manager.systemMode, Cell_Manager.IMEI, Cell_Manager.cellRDY2Use, Cell_Manager.busyReason);
		return 1;
	}
	return 0;
}
/* ====================== private functions =========================== */
/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static uint8_t CheckCellOK(void)
{
	return Cell_Manager.cellReady;
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static uint8_t CheckCellIdle(void)
{
	if(!Cell_Manager.cellReady) return 0;
//	if(Cell_Manager.RISignal) return 0;
	if(Cell_Manager.cellState != CELL_IDLE) return 0;
	
	return 1;
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static void Cell_PowerOn_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	uint8_t len;
	char *p;
	if(event == SIMCOM_EVEN_OK)
	{
		switch(Cell_Manager.step)
		{
			case 1:								
				p = (char*)ResponseBuffer;
				for(len = 0; len < 20; len++) 
				{
					if(p[len] >= '0' && p[len] <= '9')
						Cell_Manager.IMEI[len] = p[len];
					else 
					{
						Cell_Manager.IMEI[len] = 0;
						break;
					}
				}
				UARTprintf("[CELL] IMEI: %s\r\n", Cell_Manager.IMEI);
				UARTprintf("[CELL] Ready to use\r\n");
				Cell_Manager.cellRDY2Use = 1;
				//Cellular_SwitchState(CELL_IDLE);
				Cellular_SwitchState(CELL_GETBTSINFOR);
				break;		
		}
		//Cell_Manager.step++;
	}
}
/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static void Cell_Ready_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	if(event == SIMCOM_EVEN_OK)
	{
		UARTprintf("[CELL] Cellular ready\r\n");
		Cell_Manager.cellReady = 1;
		ConfigureTimer_Cellular(100);
	}
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static void Cell_Get_BTS_Info_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	//char *token[2] = {NULL, NULL};
	if(event == SIMCOM_EVEN_OK)
	{
		#if(USE_TCP_IP == 1)
		if(Cell_Mode == CELL_DATA_MODE) 
		{
			Cellular_SwitchMode(CELL_CMD_MODE);
			UARTprintf("[CELL] Switch to cmd mode ok\r\n");
			return;
		}
		#endif
		
		CPSI_Decode(ResponseBuffer, &Cell_Manager.MCC, &Cell_Manager.MNC, &Cell_Manager.LAC, &Cell_Manager.cell_ID, &Cell_Manager.RSSI, &Cell_Manager.RSRP, &Cell_Manager.RSRQ);
		if(Cell_Manager.RSRP != 0)
		{
		    Cell_Manager.BTSInfor = 1;
            //Cellular_SwitchState(CELL_IDLE);
            #if(USE_MQTT == 1)
                  if(Cell_Manager.cellState == CELL_ONLYBTSINFOR)
                  {
                      Cellular_SwitchState(CELL_IDLE);
                      Cell_Flag.Flag_Cell_GetBTInfor_Done = true;
                  }
                  else
                  {
                      Cellular_SwitchState(CELL_IDLE);
                      MQTT_Init();
                  }
            #endif
		}
		else
		{
		    Cellular_SwitchState(CELL_GETBTSINFOR);
		}

	}
	else if(event == SIMCOM_EVEN_TIMEOUT)
	{
		Cell_Manager.cellReady = 0;
		D_Flag_t.Flag_NeedToRebootNetworkModule = true;
		Cell_Manager.firstTimePowerOn = 0;
		//Cellular_SwitchState(CELL_RESET);
	}
	else if(event == SIMCOM_EVEN_ERROR)
    {
        Cell_Manager.cellReady = 0;
        D_Flag_t.Flag_NeedToRebootNetworkModule = true;
        Cell_Manager.firstTimePowerOn = 0;
        //Cellular_SwitchState(CELL_RESET);
    }
}
//--------------------------------------------------------------------------------------------------------------//
void CPSI_Decode(char* str, int* MCC, int* MNC, int* LAC, int* cell_ID, int* RSSI, int16_t* RSRSP, int16_t* RSRQ)
{
    if(strstr(str,"GSM"))
    {
        //printf("GSM CPSI decode\r\n");
        char temp_buf[50]={0};
        char _LAC[10];
        char RX_buf[10];
        int i=0, head =0, tail = 0, k=0, index=0;
        for(i=0; i < strlen(str); i++)
        {
            if(str[i] == ',') ++k;
            if(k == 1) head = i;
            if(k == 6) tail = i;//4
        }
        for(i=head+2; i <tail+1; i++)
        {
            if(str[i] == '-') str[i] = ',';
            temp_buf[index++] = str[i];
        }
        sscanf(temp_buf, "%d,%d,%[^,],%d,%[^,],,%d", MCC, MNC, _LAC, cell_ID, RX_buf, RSSI);
        *RSSI = -1 * (*RSSI);

        filter_comma(str,4,5,_LAC);
        *LAC = (int)strtol(_LAC, NULL, 0);
        //printf("End GSM CPSI decode\r\n");
    }
    if(strstr(str,"LTE"))
    {
        //SysCtlDelay(SysCtlClockGet()/10);
        //UARTprintf("[CELL] Cell info: %s\r\n", str);
        char temp_buf[50]={0};
        char _LAC[10];
        char RSRP_Buf[10];
        char RSRQ_Buf[10];
        int i = 0, head1=0, tail1=0, head2=0, tail2=0, k=0, index = 0;
        for(i=0; i<strlen(str); i++)
        {
            if(str[i] == ',') ++k;
            if(k == 1) head1 = i;
            if(k == 4) tail1 = i;
            if(k == 10) head2 = i;
            if(k == 11) tail2 = i;
        }
        for(i=head1+2; i<tail1+1; i++)
        {
            if(str[i] == '-') str[i] = ',';
            temp_buf[index++] = str[i];
        }
        for(i=head2+1; i<tail2+1; i++)
        {
            temp_buf[index++] = str[i];
        }
        sscanf(temp_buf, "%d,%d,%[^,],%d,%d", MCC, MNC, _LAC, cell_ID, RSSI);
        *LAC = (int)strtol(_LAC, NULL, 0);

        filter_comma(str, 11, 12, RSRQ_Buf);
        *RSRQ = atoi(RSRQ_Buf);
        UARTprintf("[CELL] RSRQ: %s\r\n", RSRQ_Buf);

        filter_comma(str, 12, 13, RSRP_Buf);
        *RSRSP = atoi(RSRP_Buf);
        UARTprintf("[CELL] RSRP: %s\r\n", RSRP_Buf);
        //UARTprintf("End LTE CPSI decode\r\n");
    }
}
//--------------------------------------------------------------------------------------------------------------//
int filter_comma(char *respond_data, int begin, int end, char *output)
{
    memset(output, 0, strlen(output));
    int count_filter = 0, lim = 0, start = 0, finish = 0,i;
    for (i = 0; i < strlen(respond_data); i++)
    {
        if ( respond_data[i] == ',')
        {
            count_filter ++;
            if (count_filter == begin)          start = i+1;
            if (count_filter == end)            finish = i;
        }

    }
    lim = finish - start;
    for (i = 0; i < lim; i++){
        output[i] = respond_data[start];
        start ++;
    }
    output[i] = 0;
    return 0;
}
//--------------------------------------------------------------------------------------------------------------//
void ConfigureTimer_Cellular(uint16_t timer_tick_ms)
{

    if(Cell_Manager.Cell_Timer_Init == false)
    {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
        TimerConfigure(TIMER3_BASE, TIMER_CFG_PERIODIC);
        TimerLoadSet(TIMER3_BASE, TIMER_A, (SysCtlClockGet() / (1000/timer_tick_ms)) -1);
        IntEnable(INT_TIMER3A);
        TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
        IntMasterEnable();
        Cell_Manager.Cell_Timer_Init = true;
    }
    TimerEnable(TIMER3_BASE, TIMER_A);
}
//--------------------------------------------------------------------------------------------------------------//
uint16_t timer_counter = 0;
void TimerCellularIntHandler(void)
{
    TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
    timer_counter++;
    if(timer_counter >= 10)
    {
        timer_counter = 0;
        Cellular_ManagerTask();
    }
}
