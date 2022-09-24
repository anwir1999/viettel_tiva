/*
 * MQTT_HwLayer.h
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

#ifndef _MQTT_HWLAYER_H_
#define _MQTT_HWLAYER_H_


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "../NetworkModule/Common.h"
#include "../NetworkModule/AT_Function.h"
#include "../Cellular/cellular.h"
//#include "All_User_Lib.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

typedef enum
{
    FL_MQTT_GNSS = 0,
    FL_MQTT_IMG,
    FL_MQTT_PING,
    FL_MQTT_RFID,
} FL_MQTT_DataType_t;

/* #define and enum statements go here */
#define TOPIC_DATA				"chungnt/data"		// Pub
#define TOPIC_EVENT				"chungnt/event"		// Pub
#define TOPIC_CONTROL			"chungnt/control"   // Sub
#define TOPIC_UPDATE			"chungnt/update"	// Sub

#define INNOWAY_TOPIC_WFC               "messages/%s/wificell"          // Pub
#define INNOWAY_TOPIC_GPS               "messages/%s/gps"               // Pub
#define INNOWAY_TOPIC_FLEET             "messages/%s/info"              // Pub
#define INNOWAY_TOPIC_DEVCONF           "messages/%s/devconf"           // Pub
#define INNOWAY_TOPIC_PING              "messages/%s/ping"              // Pub
#define INNOWAY_TOPIC_RFID              "messages/%s/rf"               // Pub
#define INNOWAY_TOPIC_SUB               "messages/%s/control"           // Sub

#define TEST_IMG_TOPIC                  "messages/%s/img"               // Pub
//#define TEST_IMG_TOPIC                  "img_test"                    // Pub

/* ==================================================================== */
#define INNOWAY_SERVER      1
#define TEST_SERVER         0
#define SERVER              INNOWAY_SERVER

#define TEST_MQTT_CLIENT_NAME            "Vehicle-02"
#define TEST_MQTT_CLI_IDX                0
#define TEST_MQTT_IPADD                  "52.32.182.17"
#define TEST_MQTT_PORT                   1883
#define TEST_MQTT_KEEP_TIME              60
#define TEST_MQTT_CLEAN_SESSION          1
#define TEST_MQTT_QOS                    1
#define TEST_MQTT_USERNAME               "VTAG_admin"
#define TEST_MQTT_PASSWORD               "123456a@"

#define INNOWAY_MQTT_CLIENT_NAME            "Vehicle-02"
#define INNOWAY_MQTT_CLI_IDX                0
//#define INNOWAY_MQTT_IPADD                  "171.244.133.251"
#define INNOWAY_MQTT_IPADD                  "125.212.248.229"
#define INNOWAY_MQTT_PORT                   1883
#define INNOWAY_MQTT_KEEP_TIME              60
#define INNOWAY_MQTT_CLEAN_SESSION          1
#define INNOWAY_MQTT_QOS                    1
#define INNOWAY_MQTT_USERNAME               "vinacap"
#define INNOWAY_MQTT_PASSWORD               "9gA0K35hW8QlMq0BHD08svxBdfn2PRrH"

#if SERVER // INNOWAY SERVER
    #define DEFAULT_MQTT_CLIENT_NAME            INNOWAY_MQTT_CLIENT_NAME
    #define DEFAULT_MQTT_CLI_IDX                INNOWAY_MQTT_CLI_IDX
    #define DEFAULT_MQTT_IPADD                  INNOWAY_MQTT_IPADD
    #define DEFAULT_MQTT_PORT                   INNOWAY_MQTT_PORT
    #define DEFAULT_MQTT_KEEP_TIME              INNOWAY_MQTT_KEEP_TIME
    #define DEFAULT_MQTT_CLEAN_SESSION          INNOWAY_MQTT_CLEAN_SESSION
    #define DEFAULT_MQTT_QOS                    INNOWAY_MQTT_QOS
    #define DEFAULT_MQTT_USERNAME               INNOWAY_MQTT_USERNAME
    #define DEFAULT_MQTT_PASSWORD               INNOWAY_MQTT_PASSWORD
#else // TEST SERVER
    #define DEFAULT_MQTT_CLIENT_NAME            TEST_MQTT_CLIENT_NAME
    #define DEFAULT_MQTT_CLI_IDX                TEST_MQTT_CLI_IDX
    #define DEFAULT_MQTT_IPADD                  TEST_MQTT_IPADD
    #define DEFAULT_MQTT_PORT                   TEST_MQTT_PORT
    #define DEFAULT_MQTT_KEEP_TIME              TEST_MQTT_KEEP_TIME
    #define DEFAULT_MQTT_CLEAN_SESSION          TEST_MQTT_CLEAN_SESSION
    #define DEFAULT_MQTT_QOS                    TEST_MQTT_QOS
    #define DEFAULT_MQTT_USERNAME               TEST_MQTT_USERNAME
    #define DEFAULT_MQTT_PASSWORD               TEST_MQTT_PASSWORD
#endif

#define PUBLISH_DONE 		0
#define PUBLISH_BUSY	    1
#define PUBLISH_ERROR		2
#define PUBLISH_TIMEOUT     3

#define SUBSCRIBE_DONE        0
#define SUBSCRIBE_BUSY        1
#define SUBSCRIBE_ERROR       2
#define SUBSCRIBE_TIMEOUT     3

#define MQTT_RX_LEN 		1024
#define MQTT_TX_LEN 		2048
#define MQTT_TOPIC_LEN      100
#define MQTT_ACK_TIMEOUT    10

//#define DEVICE_ID       "78639604-8276-480d-8962-65a54f3833d2" // Vehicle-03
#define DEVICE_ID       "48505448-5c50-4a47-aad3-7c7c8e6ec378" // Vehicle-02

typedef enum
{
	MQTT_STT_DISCONNECTED = 0,
	MQTT_STT_CONNECTED,
	MQTT_STT_DISCONNECT_REQUIRE,
	MQTT_STT_DISCONNECTTING,
	MQTT_STT_DISCONNECT_DONE,
} MQTT_CONNECT_STT_E;

typedef struct
{
	uint8_t connect				        : 3;
	uint8_t needConnect 	            : 1;
	uint8_t connectStatus               : 1;
	uint8_t step				        : 3;
	
	uint8_t mesIsBeingSent;
	uint8_t retryConnection;
	uint16_t timeoutConnection;	
	uint16_t timeoutPublish;
	uint8_t sub_step;
	uint8_t pub_step;
	bool flag_mqtt_ack_done;
	bool flag_mqtt_ack_wait;
	bool flag_mqtt_connection_ready;
	uint16_t mqtt_ack_timeout_Counter;
} Mqtt_Manager_t;

typedef struct
{
	char RXBuf[MQTT_RX_LEN];
	uint16_t RXlen;
	char TXBuf[MQTT_TX_LEN];
	uint16_t TXlen;
	char bufContent[50];
	/*======================*/
	uint8_t acquireClient[20];
	uint8_t clientIdx;
	char serverAdd[30];
	uint16_t port; 
	uint8_t keepAliveTime;
	uint8_t cleanSession;
	uint8_t qos;
	uint8_t username[20]; // max 256bytes
	uint8_t password[20]; // max 256bytes
	char pubTopic_Str[100];
} Mqtt_Params_t;
/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* Definition of public (external) data types go here */


/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/* Function prototypes for public (external) functions go here */
void    MQTT_PubMessage(void);
void    MQTT_ManagerTask(void);
void    MQTT_Init(void);
void    MQTT_Connect(void);
void    MQTT_Retry(void);
void    MQTT_RetryConnect(void);
void    MQTT_RestartSevice(void);
uint8_t MQTT_PublishData(const char *topic, uint8_t qos, const char *data);
uint8_t MQTT_Check_PublishStt(void);
void    MQTT_PublishRetry(void);
uint8_t MQTT_Check_ConnectStt(void);
void    MQTT_Client_Disconnect_Passively(void *ResponseBuffer);
void    MQTT_PingNetwork();
void    MQTT_SubReceive_Wait();
void    MQTT_SubACK_Callback();
void    MQTT_RFID_Publish(void);
void    MQTT_WaitForACK_Handler(void);
void    Sub_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);

static void MQTT_ReleaseClient_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void MQTTServer_Disconnect_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void MQTT_ReleaseClient_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void MQTT_ServiceStop_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
static void NetworkDisconnect(void);
void MQTT_LostConnectionHandler_Callback();
void A7672S_RestartModule(void);
static void A7672S_RestartModule_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
#endif
#ifdef __cplusplus
}
#endif
