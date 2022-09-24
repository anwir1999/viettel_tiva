/*
 * MQTT_HwLayer.c
 *
 *  Created on: Nov 01, 2020
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "MQTT_HwLayer.h"
#include "../NetworkModule/GNSS.h"
#include "../SD/SD.h"
#include "../RFID/RFID.h"


/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

#define AUTO_CONNECT	1

#define DEFAULT_TIMEOUT_CONNECT 180 // 180s
#define DEFAULT_TIMEOUT_PUBLISH 30	// 30s
#define DEFAULT_RETRY_CONNECT		5		// 5times

#define TIME_GET_SIGNAL 3 // 3s
#define RETRY			 			0
#define CANCEL		   		(TIME_GET_SIGNAL+1)

/* Default publish timeout is 60s */
#define MQTT_START_SERVICE 		"AT+CMQTTSTART\r\n"																// Start MQTT service
#define MQTT_ACQUIRE_CLIENT 	"AT+CMQTTACCQ=%d,\"%s\"\r\n"											        // Acquire a client %d: client index, %s: client name
#define MQTT_CONNECT_SERVER 	"AT+CMQTTCONNECT=%d,\"tcp://%s:%d\",%d,%d\r\n"                                  // Connect to MQTT server %d: client index, %s: server add, %d: Port, %d: keep alive time, %d: clean session
#define MQTT_INPUT_PUB_TOPIC 	"AT+CMQTTTOPIC=%d,%d\r\n"													    // Input the topic of publish message %d: client index, %d: topic length
#define MQTT_INPUT_PUB_MES 		"AT+CMQTTPAYLOAD=%d,%d\r\n"												        // Input the publish message %d: client index, %d: message length
#define MQTT_PUB_MES 			"AT+CMQTTPUB=%d,%d,60,0\r\n"											        // Publish a message to server %d: client index, %d: qos
#define MQTT_INPUT_SUB_TOPIC 	"AT+CMQTTSUBTOPIC=%d,%d,%d\r\n"										            // Input the topic of subscribe message %d: client index, %d: topic length, %d: qos
#define MQTT_SUB_MES 			"AT+CMQTTSUB=%d\r\n"													        // Subscribe a message to server %d: client index
#define MQTT_DISCONN_SERVER 	"AT+CMQTTDISC=%d,60\r\n"													    // Disconnect from server %d: client index
#define MQTT_RELEASE_CLIENT 	"AT+CMQTTREL=%d\r\n"															// Release the client %d: client index
#define MQTT_STOP_SERVICE 		"AT+CMQTTSTOP\r\n"																// Stop MQTT Service
// INNOWAY MQTT
#define INNOWAY_MQTT_START_SERVICE      "AT+CMQTTSTART\r\n"                                                             // Start MQTT service
#define INNOWAY_MQTT_ACQUIRE_CLIENT     "AT+CMQTTACCQ=%d,\"%s\"\r\n"                                                    // Acquire a client %d: client index, %s: client name
//#define INNOWAY_MQTT_CONNECT_SERVER     "AT+CMQTTCONNECT=%d,\"tcp://%s:%d\",%d,%d\r\n"                                  // Connect to MQTT server %d: client index, %s: server add, %d: Port, %d: keep alive time, %d: clean session
#define INNOWAY_MQTT_CONNECT_SERVER     "AT+CMQTTCONNECT=%d,\"tcp://%s:%d\",%d,%d,\"%s\",\"%s\"\r\n"
#define INNOWAY_MQTT_INPUT_PUB_TOPIC    "AT+CMQTTTOPIC=%d,%d\r\n"                                                       // Input the topic of publish message %d: client index, %d: topic length
#define INNOWAY_MQTT_INPUT_PUB_MES      "AT+CMQTTPAYLOAD=%d,%d\r\n"                                                     // Input the publish message %d: client index, %d: message length
#define INNOWAY_MQTT_PUB_MES            "AT+CMQTTPUB=%d,%d,60\r\n"                                                    // Publish a message to server %d: client index, %d: qos
#define INNOWAY_MQTT_INPUT_SUB_TOPIC    "AT+CMQTTSUBTOPIC=%d,%d,%d\r\n"                                                 // Input the topic of subscribe message %d: client index, %d: topic length, %d: qos
#define INNOWAY_MQTT_SUB_MES            "AT+CMQTTSUB=%d\r\n"                                                            // Subscribe a message to server %d: client index
#define INNOWAY_MQTT_DISCONN_SERVER     "AT+CMQTTDISC=%d,60\r\n"                                                        // Disconnect from server %d: client index
#define INNOWAY_MQTT_RELEASE_CLIENT     "AT+CMQTTREL=%d\r\n"                                                            // Release the client %d: client index
#define INNOWAY_MQTT_STOP_SERVICE       "AT+CMQTTSTOP\r\n"                                                              // Stop MQTT Service

/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

extern Device_Flag D_Flag_t;
extern SIMCOM_ResponseEvent_t AT_RX_event;
extern GNSS_Manager_t GNSS_Manager;
extern SD_Param_t SD_Param;
extern RFID_Manager_t RFID_Manager;
extern RFID_Data_t RFID_Data;
extern char g_dataTmpBuf[1000];

uint8_t MQTT_MsgType = FL_MQTT_GNSS;
uint16_t MQTT_Sub_Timeout_Counter = 0;

/* Global variables definitions go here */
Mqtt_Manager_t Mqtt_Manager = {.connect = CANCEL, .needConnect = 0, .connectStatus = 0, .timeoutConnection = 0, .mesIsBeingSent = false};
Mqtt_Params_t Mqtt_Params;

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* Definition of private datatypes go here */
static char bufTopic[MQTT_TOPIC_LEN];
static char SubTopic_Str[MQTT_TOPIC_LEN];
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */

/* Function prototypes for private (static) functions go here */
static uint8_t checkCRC(const char *buf, const char *rest, uint8_t errPos); // command result code
static void StartService_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void AcquireClient_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void Conn2MqttServer_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void InputTopic2Pub_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void InputMes2Pub_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void Pub_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void InputTopic2Sub_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
//static void Sub_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void DisConnMqttServer_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void ReleaseClient_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void StopService_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */

/* Functions definitions go here, organised into sections */
/*********************************************************************************************************//**
 * @brief   : handle every 1s
 * @param
 * @retval  
 ************************************************************************************************************/
void MQTT_ManagerTask(void)
{
	#if(AUTO_CONNECT == 1)
	static uint8_t autoConn = 1;
	if(autoConn == 1)
	{
		autoConn = 0;
		MQTT_Connect();
	}
	#endif
	if(Cellular_CheckReady2Use() == 1)
	{
		if(Mqtt_Manager.connect < TIME_GET_SIGNAL)
		{
			if(Cellular_CheckSignalInfo() == 0) 
			{
				if(++Mqtt_Manager.connect >= TIME_GET_SIGNAL) 
				{
					UARTprintf(".");
					Mqtt_Manager.connect = RETRY;
					Cellular_GetSignalLevel();
				}
			}
			else 
			{
				Mqtt_Manager.connect = CANCEL;
				Mqtt_Manager.needConnect = 1;
				Cellular_ClearSignalInfo();
			}
		}
	}
	
	if(Mqtt_Manager.needConnect == 1)
	{
		UARTprintf("[MQTT] connecting\r\n");
		Mqtt_Manager.needConnect = 0;
		MQTT_Init();
	}
	
	if(Mqtt_Manager.timeoutConnection > 0)
	{
		if(--Mqtt_Manager.timeoutConnection == 0)
		{
			if(Cellular_GetBusyReason() == CELL_BUSY_MQTT)
			    Cellular_SwitchState(CELL_IDLE);
			Mqtt_Manager.connect = CANCEL;
			UARTprintf("[MQTT] Cancel connect\r\n");
		}
	}
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
void MQTT_Connect(void)
{
	if(Mqtt_Manager.connectStatus == MQTT_STT_DISCONNECTED)
	{
		Mqtt_Manager.timeoutConnection = DEFAULT_TIMEOUT_CONNECT;
		Mqtt_Manager.connect = RETRY;
		Mqtt_Manager.retryConnection = DEFAULT_RETRY_CONNECT;
	}
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
void MQTT_RetryConnect(void)
{
    MQTT_MsgType = FL_MQTT_PING;
    Mqtt_Manager.mesIsBeingSent = PUBLISH_DONE;
    Mqtt_Manager.connectStatus = MQTT_STT_DISCONNECTTING;
    NetworkDisconnect();
}

void MQTT_Retry(void)
{
	if(Mqtt_Manager.connectStatus == MQTT_STT_DISCONNECTED)
	{
		if(Mqtt_Manager.retryConnection > 0)
		{
			UARTprintf("[MQTT] Retry connect to server [%d]\r\n", Mqtt_Manager.retryConnection);
			Mqtt_Manager.retryConnection--;	
			Mqtt_Manager.connect = RETRY;
		}
		else
		{
			Mqtt_Manager.timeoutConnection = 0;
			Mqtt_Manager.connect = CANCEL;
			Cellular_SwitchState(CELL_IDLE);
			UARTprintf("[MQTT] Cancel retry connect\r\n");
		}
	}
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
void MQTT_RestartSevice(void)
{
	////GPIO_WriteOutBits(CELL_LED_PORT, CELL_LED_PIN, LED_OFF);
	Mqtt_Manager.connectStatus = MQTT_STT_DISCONNECTED;
	Mqtt_Manager.timeoutConnection = DEFAULT_TIMEOUT_CONNECT;
	Mqtt_Manager.connect = RETRY;
	Mqtt_Manager.retryConnection = DEFAULT_RETRY_CONNECT;
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
void MQTT_Init(void)
{
    Mqtt_Manager.flag_mqtt_connection_ready = false;
    Mqtt_Manager.flag_mqtt_ack_done = false;
    //Mqtt_Manager.flag_mqtt_ack_done = true;

    MQTT_MsgType = FL_MQTT_PING;
	Cellular_Change2BusyState(CELL_BUSY_MQTT);
	
    // Reset SD flag
    SD_Param.Flag_SD_Data_Begin = true;
    SD_Param.Flag_SD_Data_Block_Request = false;
    SD_Param.Flag_SD_Data_End = false;
    SD_Param.Flag_SD_Data_Ready = false;
    SD_Param.Flag_SD_Data_Request = false;

	Mqtt_Manager.connectStatus = MQTT_STT_DISCONNECTED;

	//EN25Q_readData(FLASH_MQTT_AREA, &Mqtt_Params, sizeof(Mqtt_Params));
	//if(Mqtt_Params.port == 0xFFFF)
	{
		memcpy(Mqtt_Params.acquireClient, DEFAULT_MQTT_CLIENT_NAME, sizeof(DEFAULT_MQTT_CLIENT_NAME));
		Mqtt_Params.clientIdx = DEFAULT_MQTT_CLI_IDX;
		memcpy(Mqtt_Params.serverAdd, DEFAULT_MQTT_IPADD, sizeof(DEFAULT_MQTT_IPADD));
		Mqtt_Params.port = DEFAULT_MQTT_PORT;
		Mqtt_Params.keepAliveTime = DEFAULT_MQTT_KEEP_TIME;
		Mqtt_Params.cleanSession = DEFAULT_MQTT_CLEAN_SESSION;
		Mqtt_Params.qos = DEFAULT_MQTT_QOS;
		memcpy(Mqtt_Params.username, DEFAULT_MQTT_USERNAME, sizeof(DEFAULT_MQTT_USERNAME));
		memcpy(Mqtt_Params.password, DEFAULT_MQTT_PASSWORD, sizeof(DEFAULT_MQTT_PASSWORD));
	}

	Simcom_ATC_SendATCommand(MQTT_START_SERVICE, "+CMQTTSTART:", 10000, 5, StartService_Callback);
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
uint8_t MQTT_PublishData(const char *topic, uint8_t qos, const char *data)
{
	if((Mqtt_Manager.connectStatus == MQTT_STT_CONNECTED) && (Mqtt_Manager.mesIsBeingSent == PUBLISH_DONE))
	{
		uint16_t topicLen;
		Mqtt_Manager.mesIsBeingSent = PUBLISH_BUSY;
		
		Cellular_Change2BusyState(CELL_BUSY_MQTT);
		
		//UARTprintf("[MQTT] Pub to: %s, data: %s\r\n", topic, data);
		UARTprintf("[MQTT] Pub to: %s\r\n", topic);
		
		/* Input the topic of publish message */
		memset(bufTopic, 0, sizeof(bufTopic));
		topicLen = sprintf(bufTopic, "%s\r\n", topic) - 2;
		
		/* Input the publish message */
		memset(Mqtt_Params.TXBuf, 0, sizeof(Mqtt_Params.TXBuf));
		Mqtt_Params.TXlen = sprintf(Mqtt_Params.TXBuf, "%s", data);
		Mqtt_Params.qos = qos;
		

		memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
		sprintf(Mqtt_Params.bufContent, MQTT_INPUT_PUB_TOPIC, DEFAULT_MQTT_CLI_IDX, topicLen);
		//sprintf(Mqtt_Params.bufContent, MQTT_INPUT_PUB_TOPIC, Mqtt_Params.clientIdx, topicLen);
		Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, ">", 1000, 5, InputTopic2Pub_Callback);
		return 1;
	}
	else if(Mqtt_Manager.mesIsBeingSent == PUBLISH_BUSY)
	{
		UARTprintf("[MQTT] Publish busy\r\n");
	}
	return 0;
}

/*********************************************************************************************************//**
 * @brief   check publish status: busy or done
 * @paramtf
 * @retval  
 ************************************************************************************************************/
uint8_t MQTT_Check_PublishStt(void)
{
	return Mqtt_Manager.mesIsBeingSent;
}

/*********************************************************************************************************//**
 * @brief   retry to publish
 * @paramtf
 * @retval  
 ************************************************************************************************************/
void MQTT_PublishRetry(void)
{
	Mqtt_Manager.mesIsBeingSent = PUBLISH_DONE;
}

/*********************************************************************************************************//**
 * @brief   check connection
 * @paramtf
 * @retval  
 ************************************************************************************************************/
uint8_t MQTT_Check_ConnectStt(void)
{
	return Mqtt_Manager.connectStatus;
}

/*********************************************************************************************************//**
 * Unsolicited Result Codes 
 ************************************************************************************************************/

/*********************************************************************************************************//**
 * @brief   +CMQTTCONNLOST: clientIndx,cause
 * @paramtf
 * @retval  
 ************************************************************************************************************/
void MQTT_Client_Disconnect_Passively(void *ResponseBuffer)
{
	char *tok;
	uint8_t cindx;
	uint8_t cause;
	tok = strtok((char*)ResponseBuffer, ":");
	tok = strtok(NULL, ",");
	cindx = atoi(tok);
	tok = strtok(NULL, "\r\n");
	cause = atoi(tok);
	
	Mqtt_Manager.connectStatus = MQTT_STT_DISCONNECTED;
	//GPIO_WriteOutBits(CELL_LED_PORT, CELL_LED_PIN, LED_OFF);
	UARTprintf("[MQTT] Client Disconnect Passively, index: %d ", cindx);
	switch(cause)
	{
		case 1: UARTprintf("Socket is closed passively"); break;
		case 2: UARTprintf("Socket is reset"); break;
		case 3: UARTprintf("Network is closed"); break;
	}
	UARTprintf("\r\n");
	
	//Buzzer_SetBeep(500, 1000, 5);
	
	MQTT_Connect();
}

/*********************************************************************************************************//**
 * @brief   +CMQTTRXSTART: <client_index>,<topic_total_len>,<payload_total_len>\r\n
						\r\n
						+CMQTTRXTOPIC: <client_index>,<sub_topic_len>\r\n
						<sub_topic>\r\n
						+CMQTTRXPAYLOAD: <client_index>,<sub_payload_len>\r\n
						<sub_payload>\r\n
						+CMQTTRXEND: <client_index>\r\n
 * @paramtf
 * @retval  
 ************************************************************************************************************/
void MQTT_RevMessage(void *ResponseBuffer)
{
	static uint8_t startParse, parseDone, clientIndx;
	static uint16_t topicTotalLen, payloadTotalLen, subTopicLen;
	
	if(strstr((const char*)ResponseBuffer, "+CMQTTRXSTART"))
	{
		
	}
}

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */

/*********************************************************************************************************//**
 * @brief   : 
 * @param		: eg: buf +CMQTTSTART:0\r\n, 		 rest +CMQTTSTART, errPos 0
								  buf +CMQTTCONNECT:0,0\r\n, rest +CMQTTCONNECT, errPos 1
 * @retval  
 ************************************************************************************************************/
static uint8_t checkCRC(const char *buf, const char *rest, uint8_t errPos)
{
	char *tok, *tem;
	uint8_t err;
	tem = strstr(buf, rest);
	tok = strtok(tem, ":");
	if(errPos == 0) tok = strtok(NULL, "\r\n");
	else if(errPos == 1)
	{
		tok = strtok(NULL, ",");
		tok = strtok(NULL, "\r\n");
	}
		
	err = atoi(tok);
	UARTprintf("[MQTT] Check CRC: ");
	switch(err)
	{
		case 0:  UARTprintf("operation succeeded"); break;
		case 1:  UARTprintf("failed"); break;
		case 2:  UARTprintf("bad UTF-8 string"); break;
		case 3:  UARTprintf("sock connect fail"); break;
		case 4:  UARTprintf("sock create fail"); break;
		case 5:  UARTprintf("sock close fail"); break;
		case 6:  UARTprintf("message receive fail"); break;
		case 7:  UARTprintf("network open fail"); break;
		case 8:  UARTprintf("network close fail"); break;
		case 9:  UARTprintf("network not opened"); break;
		case 10: UARTprintf("client index error"); break;
		case 11: UARTprintf("no connection"); break;
		case 12: UARTprintf("invalid parameter"); break;
		case 13: UARTprintf("not supported operation"); break;
		case 14: UARTprintf("client is busy"); break;
		case 15: UARTprintf("equire connection fail"); break;
		case 16: UARTprintf("sock sending fail"); break;
		case 17: UARTprintf("timeout"); break;
		case 18: UARTprintf("topic is empty"); break;
		case 19: UARTprintf("client is used"); break;
		case 20: UARTprintf("client not acquired"); break;
		case 21: UARTprintf("client not released"); break;
		case 22: UARTprintf("length out of range"); break;
		case 23: UARTprintf("network is opened"); break;
		case 24: UARTprintf("packet fail"); break;
		case 25: UARTprintf("DNS error"); break;
		case 26: UARTprintf("socket is closed by server"); break;
		case 27: UARTprintf("connection refused: unaccepted protocol version"); break;
		case 28: UARTprintf("connection refused: identifier rejected"); break;
		case 29: UARTprintf("connection refused: server unavailable"); break;
		case 30: UARTprintf("connection refused: bad user name or password"); break;
		case 31: UARTprintf("connection refused: not authorized"); break;
		case 32: UARTprintf("handshake fail"); break;
		case 33: UARTprintf("not set certificate"); break;
		case 34: UARTprintf("Open session failed"); break;
		case 35: UARTprintf("Disconnect from server failed"); break;
	}
	UARTprintf("\r\n");
	return err;
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static void StartService_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	uint8_t err;
	if(event == SIMCOM_EVEN_OK)
	{
		err = checkCRC((const char*)ResponseBuffer, "+CMQTTSTART:", 0);
		if(err == 0)
		{			
			memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
			sprintf(Mqtt_Params.bufContent, MQTT_ACQUIRE_CLIENT, DEFAULT_MQTT_CLI_IDX, Mqtt_Params.acquireClient);
			Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, "OK", 2000, 5, AcquireClient_Callback);
		}
		// Todo: else
	}
	else if(event == SIMCOM_EVEN_ERROR)
	{
	    SysCtlReset();

	}
	else if(event == SIMCOM_EVEN_TIMEOUT)
	{
		//MQTT_Retry();
		SysCtlReset();
	}
}
 
/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static void AcquireClient_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	if(event == SIMCOM_EVEN_OK)
	{		
		memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
		sprintf(Mqtt_Params.bufContent, INNOWAY_MQTT_CONNECT_SERVER, DEFAULT_MQTT_CLI_IDX, Mqtt_Params.serverAdd, Mqtt_Params.port, Mqtt_Params.keepAliveTime, Mqtt_Params.cleanSession, Mqtt_Params.username, Mqtt_Params.password);
		Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, "+CMQTTCONNECT:", 20000, 3, Conn2MqttServer_Callback);
	}
	else
	{
		//MQTT_Retry();
		SysCtlReset();
	}
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static void Conn2MqttServer_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	uint8_t err;
	if(event == SIMCOM_EVEN_OK)
	{
		err = checkCRC((const char*)ResponseBuffer, "+CMQTTCONNECT:", 1);
		if(err == 0) 
		{			
			Mqtt_Manager.step = 0;
			Mqtt_Manager.connectStatus = MQTT_STT_CONNECTED;
			UARTprintf("[MQTT] Connected\r\n");
			Mqtt_Manager.timeoutConnection = 0;
			Mqtt_Manager.sub_step = 0;
			InputTopic2Sub_Callback(SIMCOM_EVEN_OK, NULL);
		}
		else
		{
		    UARTprintf("[MQTT] Connect err: %s\r\n", (char*)ResponseBuffer);
		    Mqtt_Manager.mesIsBeingSent = PUBLISH_ERROR;
		    MQTT_RetryConnect();
		    //D_Flag_t.Flag_NeedToRebootNetworkModule = true;
		}
	}
	else if(event == SIMCOM_EVEN_ERROR)
    {
        UARTprintf("[MQTT] Connect err: %s\r\n", (char*)ResponseBuffer);
        Mqtt_Manager.mesIsBeingSent = PUBLISH_ERROR;
        MQTT_RetryConnect();
        //D_Flag_t.Flag_NeedToRebootNetworkModule = true;
    }
    else if(event == SIMCOM_EVEN_TIMEOUT)
    {
        UARTprintf("[MQTT] Connect timeout: %s\r\n", (char*)ResponseBuffer);
        Mqtt_Manager.mesIsBeingSent = PUBLISH_TIMEOUT;
        MQTT_RetryConnect();
        //D_Flag_t.Flag_NeedToRebootNetworkModule = true;
    }
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static void InputTopic2Sub_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	uint8_t topicLen;
	if(event == SIMCOM_EVEN_OK)
	{
	    switch(Mqtt_Manager.sub_step)
	    {
	        case 0:
                sprintf(SubTopic_Str, INNOWAY_TOPIC_SUB, DEVICE_ID);
                topicLen = strlen(SubTopic_Str);
                memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
                sprintf(Mqtt_Params.bufContent, INNOWAY_MQTT_INPUT_SUB_TOPIC, DEFAULT_MQTT_CLI_IDX, topicLen, Mqtt_Params.qos);
                Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, ">", 1000, 5, InputTopic2Sub_Callback);
                Mqtt_Manager.sub_step = 1;
                break;
	        case 1:
	            UARTprintf("Sub topic: %s\n", SubTopic_Str);
                Simcom_ATC_SendATCommand(SubTopic_Str, "OK", 1000, 5, InputTopic2Sub_Callback);
                Mqtt_Manager.sub_step = 2;
                break;
	        case 2:
                memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
                sprintf(Mqtt_Params.bufContent, MQTT_SUB_MES, DEFAULT_MQTT_CLI_IDX);
                Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, "+CMQTTSUB:", 20000, 3, Sub_Callback);
                Mqtt_Manager.sub_step = 3;
                break;
	    }
	}
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
void Sub_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	uint8_t err;
	if(event == SIMCOM_EVEN_OK)
	{
		err = checkCRC((const char*)ResponseBuffer, "+CMQTTSUB:", 1);
		if(err == 0)
		{			
			UARTprintf("[MQTT] Sub done, ready to use\r\n");
			Cellular_SwitchState(CELL_IDLE);
			UARTprintf("MQTT_MsgType = %d \r\n", MQTT_MsgType);
			// Running GNSS locating
			if(Mqtt_Manager.connectStatus == MQTT_STT_CONNECTED)
			{
			    switch(MQTT_MsgType)
			    {
                    case FL_MQTT_GNSS:
                        break;
                    case FL_MQTT_PING:
                        MQTT_PingNetwork();
                        break;
                    case FL_MQTT_IMG:
                        // Request data from SD card only one
                        if(SD_Param.Flag_SD_Data_Begin == true)
                        {
                            SD_Param.Flag_SD_Data_Request = true;
                            SD_Param.Flag_SD_Data_Begin = false;
                        }
                        //Switch to GNSS when finish sending image
                        if(SD_Param.Flag_SD_Data_End == true)
                        {
                            SD_Param.Flag_SD_Data_End = false;
                            SD_Param.Flag_SD_Data_Block_Request = false;
                            MQTT_MsgType = FL_MQTT_GNSS;
                            Sub_Callback(SIMCOM_EVEN_OK, "+CMQTTSUB:");
                        }
                        else
                        {
                            SD_Param.Flag_SD_Data_Block_Request = true;
                            UARTprintf("[MQTT] Wait for data\r\n");
                        }

                        break;
                    default:
                        break;
                }
			}
		}
		else
		{
			MQTT_RetryConnect();
			//SysCtlReset();
		}
	}
	else
	{
	    MQTT_RetryConnect();
		//SysCtlReset();
	}
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static void DisConnMqttServer_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	uint8_t stt;
	char tem[14] = {0};
	char *tok, *buf;
	
	if(event == SIMCOM_EVEN_OK)
	{
		switch(Mqtt_Manager.step)
		{
			case 0:
				sprintf(tem, "+CMQTTDISC: %d", DEFAULT_MQTT_CLI_IDX);
				UARTprintf("tem: %s\r\n", tem);
				// +CMQTTDISC: 0,0
				buf = strstr((const char*)ResponseBuffer, tem); // +CMQTTDISC: <index>
				tok = strtok(buf, ",");
				tok = strtok(NULL, "\r\n");
				stt = atoi(tok);
				if(stt == 0) // connection
				{
					memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
					//sprintf(Mqtt_Params.bufContent, MQTT_DISCONN_SERVER, DEFAULT_MQTT_CLI_IDX);
					sprintf(Mqtt_Params.bufContent, MQTT_DISCONN_SERVER, 1);
					Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, "+CMQTTDISC:", 65000, 3, DisConnMqttServer_Callback);
				}
				else
				{
					memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
					sprintf(Mqtt_Params.bufContent, MQTT_RELEASE_CLIENT, DEFAULT_MQTT_CLI_IDX);
					Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, "OK", 1000, 5, ReleaseClient_Callback);
				}
				break;
			case 1:
				stt = checkCRC((const char*)ResponseBuffer, "+CMQTTDISC:", 1);
				if(stt == 0)
				{
					memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
					sprintf(Mqtt_Params.bufContent, MQTT_RELEASE_CLIENT, DEFAULT_MQTT_CLI_IDX);
					Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, "OK", 1000, 5, ReleaseClient_Callback);
				}
				else if(stt == 35)
				{
					// todo: Disconnect from server failed
				}
				break;
		}
		Mqtt_Manager.step++;
	}
}
/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static void ReleaseClient_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	uint8_t err;
	if(event == SIMCOM_EVEN_OK)
	{
		Simcom_ATC_SendATCommand(MQTT_STOP_SERVICE, "+CMQTTSTOP:", 65000, 5, StopService_Callback);
	}
	else if(event == SIMCOM_EVEN_OK)
	{
		err = checkCRC((const char*)ResponseBuffer, "+CMQTTREL:", 1);
		if(err == 19) 
		{
			Simcom_ATC_SendATCommand(MQTT_STOP_SERVICE, "+CMQTTSTOP:", 65000, 5, StopService_Callback);
		}
	}
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static void StopService_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	uint8_t err;
	if(event == SIMCOM_EVEN_OK)
	{
		err = checkCRC((const char*)ResponseBuffer, "+CMQTTSTOP:", 0);
		if(err == 0) 
		{			
			UARTprintf("[MQTT] Stop service ok\r\n");
			Mqtt_Manager.connectStatus = MQTT_STT_DISCONNECTED;
			Cellular_SwitchState(CELL_IDLE);
			
			MQTT_Retry();
		}
		else if(err == 17)
		{
			UARTprintf("[MQTT] Stop service timeout\r\n");
		}
	}
}
/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static void InputTopic2Pub_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	if(event == SIMCOM_EVEN_OK)
	{
		Mqtt_Manager.step = 0;
		Mqtt_Manager.pub_step = 0;
		Simcom_ATC_SendATCommand(bufTopic, "OK", 1000, 5, InputMes2Pub_Callback);
	}
	else if(event == SIMCOM_EVEN_ERROR)
	{
	    UARTprintf("[MQTT] Pub error\r\n");
	    SysCtlReset();

	}
	else if(event == SIMCOM_EVEN_TIMEOUT)
	{
	    UARTprintf("[MQTT] Pub timeout\r\n");
	    SysCtlReset();
	}
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
void MQTT_PubMessage(void)
{
    if(AT_RX_event == SIMCOM_EVEN_OK)
    {
       D_Flag_t.Flag_Wait_Exit = false;
       memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
       //sprintf(Mqtt_Params.bufContent, MQTT_INPUT_PUB_MES, Mqtt_Params.clientIdx, Mqtt_Params.TXlen);
       sprintf(Mqtt_Params.bufContent, MQTT_INPUT_PUB_MES, DEFAULT_MQTT_CLI_IDX, Mqtt_Params.TXlen);
       Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, ">", 1000, 5, Simcom_ATResponse_Callback);
       WaitandExitLoop(&D_Flag_t.Flag_Wait_Exit);
    }
    else if(AT_RX_event == SIMCOM_EVEN_ERROR || AT_RX_event == SIMCOM_EVEN_TIMEOUT)
    {
        MQTT_RetryConnect();
    }

    if(AT_RX_event == SIMCOM_EVEN_OK)
    {
         D_Flag_t.Flag_Wait_Exit = false;
         //UARTprintf("[MQTT] Pub data: %s\r\n",Mqtt_Params.TXBuf);
         Simcom_ATC_SendATCommand(Mqtt_Params.TXBuf, "OK", 1000, 5, Simcom_ATResponse_Callback);
         WaitandExitLoop(&D_Flag_t.Flag_Wait_Exit);
    }
    else if(AT_RX_event == SIMCOM_EVEN_ERROR || AT_RX_event == SIMCOM_EVEN_TIMEOUT)
    {
        MQTT_RetryConnect();
    }

    if(AT_RX_event == SIMCOM_EVEN_OK)
    {
       memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
       sprintf(Mqtt_Params.bufContent, INNOWAY_MQTT_PUB_MES, DEFAULT_MQTT_CLI_IDX, Mqtt_Params.qos);
       Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, "+CMQTTPUB:", 60000, 0, Pub_Callback);
       Mqtt_Manager.flag_mqtt_ack_done = false;
    }
    else if(AT_RX_event == SIMCOM_EVEN_ERROR || AT_RX_event == SIMCOM_EVEN_TIMEOUT)
    {
        MQTT_RetryConnect();
    }
}
/*********************************************************************************************************//**
 * @brief
 * @param
 * @retval
 ************************************************************************************************************/
static void InputMes2Pub_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
    if(event == SIMCOM_EVEN_OK)
    {
        switch (Mqtt_Manager.pub_step)
        {
            case 0:
                memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
                //sprintf(Mqtt_Params.bufContent, MQTT_INPUT_PUB_MES, Mqtt_Params.clientIdx, Mqtt_Params.TXlen);
                sprintf(Mqtt_Params.bufContent, MQTT_INPUT_PUB_MES, DEFAULT_MQTT_CLI_IDX, Mqtt_Params.TXlen);
                Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, ">", 1000, 5, InputMes2Pub_Callback);
                break;
            case 1:
                Simcom_ATC_SendATCommand(Mqtt_Params.TXBuf, "OK", 1000, 5, InputMes2Pub_Callback);
                break;
            case 2:
                memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
                //sprintf(Mqtt_Params.bufContent, MQTT_PUB_MES, Mqtt_Params.clientIdx, Mqtt_Params.qos);
                sprintf(Mqtt_Params.bufContent, INNOWAY_MQTT_PUB_MES, DEFAULT_MQTT_CLI_IDX, Mqtt_Params.qos);
                Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, "+CMQTTPUB:", 65000, 0, Pub_Callback);
            default:
                break;
        }
        Mqtt_Manager.pub_step++;
    }
    else if(event == SIMCOM_EVEN_ERROR)
    {
        UARTprintf("[MQTT] InputMes2Pub err: %s\r\n", (char*)ResponseBuffer);
        Mqtt_Manager.mesIsBeingSent = PUBLISH_DONE; // skip current message
        Cellular_SwitchState(CELL_IDLE);
        //SysCtlReset();
    }
    else if(event == SIMCOM_EVEN_TIMEOUT)
    {
        UARTprintf("[MQTT] InputMes2Pub timeout: %s\r\n", (char*)ResponseBuffer);
        Mqtt_Manager.mesIsBeingSent = PUBLISH_DONE; // skip current message
        Cellular_SwitchState(CELL_IDLE);
        //SysCtlReset();
    }
}

/*********************************************************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************************************************/
static void Pub_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
	uint8_t err;
	if(event == SIMCOM_EVEN_OK)
	{
		err = checkCRC((const char*)ResponseBuffer, "+CMQTTPUB:", 1);
		if(err == 0) 
		{			
			UARTprintf("[MQTT] Pub done\r\n");
			Mqtt_Manager.mesIsBeingSent = PUBLISH_DONE;
			Mqtt_Manager.mqtt_ack_timeout_Counter = 0;
			UARTprintf("[MQTT] MQTT_MsgType : %d\r\n", MQTT_MsgType);
			switch(MQTT_MsgType)
            {
                case FL_MQTT_GNSS:
                    GNSS_EnableReading(false);
                    //GNSS_Manager.flag_GNSS_pub_done = true;
                    //Mqtt_Manager.flag_mqtt_ack_done = false;
                    Mqtt_Manager.flag_mqtt_ack_wait = true;
                    break;
                case FL_MQTT_IMG:
                    Sub_Callback(SIMCOM_EVEN_OK, "+CMQTTSUB:");
                    break;
                case FL_MQTT_PING:
                    //Mqtt_Manager.flag_mqtt_ack_done = false;
                    Mqtt_Manager.flag_mqtt_ack_wait = true;
                    break;
                case FL_MQTT_RFID:
                    //Mqtt_Manager.flag_mqtt_ack_done = false;
                    Mqtt_Manager.flag_mqtt_ack_wait = true;
                    break;
                default:
                    break;
            }
		}
		else if(err == 17)
		{
			UARTprintf("[MQTT] Pub timeout\r\n");
			Mqtt_Manager.mesIsBeingSent = PUBLISH_TIMEOUT;
			MQTT_RetryConnect();
			//D_Flag_t.Flag_NeedToRebootNetworkModule = true;
			//SysCtlReset();
		}
		else if(err == 18)
		{
			UARTprintf("[MQTT] Pub no topic\r\n");
			Mqtt_Manager.mesIsBeingSent = PUBLISH_ERROR;
			MQTT_RetryConnect();
			//D_Flag_t.Flag_NeedToRebootNetworkModule = true;
			//SysCtlReset();
		}
	}
	else if(event == SIMCOM_EVEN_ERROR)
	{
		UARTprintf("[MQTT] Pub err: %s\r\n", (char*)ResponseBuffer);
		Mqtt_Manager.mesIsBeingSent = PUBLISH_ERROR;
		MQTT_RetryConnect();
		//D_Flag_t.Flag_NeedToRebootNetworkModule = true;
		//SysCtlReset();
	}
	else if(event == SIMCOM_EVEN_TIMEOUT)
	{
		UARTprintf("[MQTT] Pub timeout: %s\r\n", (char*)ResponseBuffer);
		Mqtt_Manager.mesIsBeingSent = PUBLISH_TIMEOUT;
		MQTT_RetryConnect();
		//D_Flag_t.Flag_NeedToRebootNetworkModule = true;
		//SysCtlReset();
	}
	
	Cellular_SwitchState(CELL_IDLE);
}
/*********************************************************************************************************//**
 * @brief
 * @param
 * @retval
 ************************************************************************************************************/
static void NetworkDisconnect(void)
{
    memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
    sprintf(Mqtt_Params.bufContent, MQTT_DISCONN_SERVER, 0);
    Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, "+CMQTTDISC:", 65000, 3, MQTTServer_Disconnect_Callback);
}
/*********************************************************************************************************//**
 * @brief
 * @param
 * @retval
 ************************************************************************************************************/
static void MQTTServer_Disconnect_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
    if(event == SIMCOM_EVEN_OK)
    {
        UARTprintf("[MQTT] Disconnect server: SIMCOM_EVENT_OK\r\n");
        memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
        sprintf(Mqtt_Params.bufContent, INNOWAY_MQTT_RELEASE_CLIENT, 0);
        Simcom_ATC_SendATCommand(Mqtt_Params.bufContent, "OK", 65000, 3, MQTT_ReleaseClient_Callback);
    }
    else if(event == SIMCOM_EVEN_ERROR)
    {
        UARTprintf("[MQTT] Disconnect server: SIMCOM_EVENT_ERROR\r\n");
        SysCtlReset();
    }
    else if(event == SIMCOM_EVEN_TIMEOUT)
    {
        UARTprintf("[MQTT] Disconnect server: SIMCOM_EVENT_TIMEOUT\r\n");
        SysCtlReset();
    }
}
/*********************************************************************************************************//**
 * @brief
 * @param
 * @retval
 ************************************************************************************************************/
static void MQTT_ReleaseClient_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
    if(event == SIMCOM_EVEN_OK)
    {
        UARTprintf("[MQTT] Release Client: SIMCOM_EVENT_OK\r\n");
        Simcom_ATC_SendATCommand(INNOWAY_MQTT_STOP_SERVICE, "+CMQTTSTOP:", 65000, 3, MQTT_ServiceStop_Callback);
    }
    else if(event == SIMCOM_EVEN_ERROR)
    {
        UARTprintf("[MQTT] Release Client: SIMCOM_EVENT_ERROR\r\n");
        SysCtlReset();
    }
    else if(event == SIMCOM_EVEN_TIMEOUT)
    {
        UARTprintf("[MQTT] Release Client: SIMCOM_EVENT_TIMEOUT\r\n");
        SysCtlReset();
    }
}
/*********************************************************************************************************//**
 * @brief
 * @param
 * @retval
 ************************************************************************************************************/
static void MQTT_ServiceStop_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
    if(event == SIMCOM_EVEN_OK)
    {
        UARTprintf("[MQTT] Stop service: SIMCOM_EVENT_OK\r\n");
        Mqtt_Manager.connectStatus = MQTT_STT_DISCONNECTED;
        Cellular_SwitchState(CELL_IDLE);
        MQTT_Init();
    }
    else if(event == SIMCOM_EVEN_ERROR)
    {
        UARTprintf("[MQTT] Stop service: SIMCOM_EVENT_ERROR\r\n");
        Cellular_SwitchState(CELL_IDLE);
        SysCtlReset();
    }
    else if(event == SIMCOM_EVEN_TIMEOUT)
    {
        UARTprintf("[MQTT] Stop service: SIMCOM_EVENT_TIMEOUT\r\n");
        Cellular_SwitchState(CELL_IDLE);
        SysCtlReset();
    }
}
/*********************************************************************************************************//**
 * @brief
 * @param
 * @retval
 ************************************************************************************************************/
void A7672S_RestartModule(void)
{
    Simcom_ATC_SendATCommand("AT+CFUN=7\r\n","OK", 1000, 5, A7672S_RestartModule_Callback);
}
/*********************************************************************************************************//**
 * @brief
 * @param
 * @retval
 ************************************************************************************************************/
static void A7672S_RestartModule_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
    if(event == SIMCOM_EVEN_OK)
    {
        UARTprintf("[RESTART] Offline mode: SIMCOM_EVENT_OK\r\n");
        Simcom_ATC_SendATCommand("AT+CFUN=6\r\n","OK", 1000, 5, Simcom_ATResponse_Callback);
        Cellular_SwitchState(CELL_IDLE);
        //SysCtlReset();
    }
    else if(event == SIMCOM_EVEN_ERROR)
    {
        UARTprintf("[RESTART] Offline mode: SIMCOM_EVENT_ERROR\r\n");
        Cellular_SwitchState(CELL_IDLE);
    }
    else if(event == SIMCOM_EVEN_TIMEOUT)
    {
        UARTprintf("[RESTART] Offline mode: SIMCOM_EVENT_TIMEOUT\r\n");
        Cellular_SwitchState(CELL_IDLE);
    }
}
/*********************************************************************************************************//**
 * @brief
 * @param
 * @retval
 ************************************************************************************************************/
void MQTT_LostConnectionHandler_Callback()
{
    Mqtt_Manager.connectStatus = MQTT_STT_DISCONNECTED;
    GNSS_EnableReading(false);
    AcquireClient_Callback(SIMCOM_EVEN_OK, "");
}
/*********************************************************************************************************//**
 * @brief
 * @param
 * @retval
 ************************************************************************************************************/
void MQTT_PingNetwork()
{
    UARTprintf("Ping the network ... \r\n");
    // Publish data through MQTT
    memset(Mqtt_Params.pubTopic_Str, 0, sizeof(Mqtt_Params.pubTopic_Str));
    sprintf(Mqtt_Params.pubTopic_Str, INNOWAY_TOPIC_PING, DEVICE_ID);
    MQTT_PublishData(Mqtt_Params.pubTopic_Str, 1, "{\"code\":0}");
}
/*********************************************************************************************************//**
 * @brief
 * @param
 * @retval
 ************************************************************************************************************/
void MQTT_SubReceive_Wait()
{
    if(Mqtt_Manager.mqtt_ack_timeout_Counter <= MQTT_ACK_TIMEOUT)
    {
        Mqtt_Manager.mqtt_ack_timeout_Counter++;
        if(Mqtt_Manager.flag_mqtt_ack_done == true)
        {
            Mqtt_Manager.mqtt_ack_timeout_Counter = MQTT_ACK_TIMEOUT + 1;
            Mqtt_Manager.flag_mqtt_ack_done = false;
            switch(MQTT_MsgType)
            {
                case FL_MQTT_GNSS:
                    break;
                case FL_MQTT_IMG:
                    break;
                case FL_MQTT_PING:
                    RFID_Task_Execute(true);
                case FL_MQTT_RFID:
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        SysCtlReset();
    }
    //Mqtt_Manager.mqtt_ack_timeout_Counter++;
}
/*********************************************************************************************************//**
 * @brief
 * @param
 * @retval
 ************************************************************************************************************/
void MQTT_SubACK_Callback()
{
    Mqtt_Manager.flag_mqtt_ack_done = true;
    //Mqtt_Manager.flag_mqtt_ack_wait = false;
}
/*********************************************************************************************************//**
 * @brief
 * @param
 * @retval
 ************************************************************************************************************/
void MQTT_RFID_Publish(void)
{
    RFID_Manager.Flag_RFID_SD_Store_Done = false;
    // Publish data through MQTT
    memset(Mqtt_Params.pubTopic_Str, 0, sizeof(Mqtt_Params.pubTopic_Str));
    sprintf(Mqtt_Params.pubTopic_Str, INNOWAY_TOPIC_RFID, DEVICE_ID);
    MQTT_PublishData(Mqtt_Params.pubTopic_Str, 1, RFID_Data.RFID_Data_Str);
}

void MQTT_WaitForACK_Handler(void)
{
    if(Mqtt_Manager.flag_mqtt_ack_wait == true)
    {
       if(Mqtt_Manager.mqtt_ack_timeout_Counter <= MQTT_ACK_TIMEOUT)
       {
           Mqtt_Manager.mqtt_ack_timeout_Counter++;
           UARTprintf("[MQTT] Subscribe ACK waiting time: %d s\r\n", Mqtt_Manager.mqtt_ack_timeout_Counter);
           if(Mqtt_Manager.flag_mqtt_ack_done == true)
           {
               UARTprintf("[MQTT] Subscribe ACK wait successful \r\n");
               Mqtt_Manager.mqtt_ack_timeout_Counter = MQTT_ACK_TIMEOUT + 1;
               Mqtt_Manager.flag_mqtt_ack_done = false;
               Mqtt_Manager.flag_mqtt_ack_wait = false;
               if(MQTT_MsgType == FL_MQTT_PING)
               {
                   Mqtt_Manager.flag_mqtt_connection_ready = true;
                   MQTT_MsgType = FL_MQTT_RFID;
                   // Run RFID task
                   RFID_Task_Execute(true);
               }
               else if(MQTT_MsgType == FL_MQTT_RFID)
               {
                   Mqtt_Manager.flag_mqtt_connection_ready = true;
                   RFID_Manager.Flag_RFID_Sending_Inprocess = false;
                   // Run GNSS task
                   MQTT_MsgType = FL_MQTT_GNSS;
                   GNSS_Task_Execute();
               }
               else if(MQTT_MsgType == FL_MQTT_GNSS)
               {
                   Mqtt_Manager.flag_mqtt_connection_ready = true;
                   GNSS_Manager.flag_GNSS_sending_inprocess = false;
                   MQTT_MsgType = FL_MQTT_RFID;
               }
           }
       }
       else
       {
           UARTprintf("[MQTT] Subscribe ACK wait timeout \r\n");
           Mqtt_Manager.flag_mqtt_connection_ready = false;
           Mqtt_Manager.flag_mqtt_ack_done = false;
           Mqtt_Manager.flag_mqtt_ack_wait = false;
           Mqtt_Manager.mqtt_ack_timeout_Counter = 0;

           MQTT_RetryConnect();
       }
    }
}
