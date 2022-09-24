/*
 * RFID.h
 *
 *  Created on: Feb 22, 2022
 *      Author: QuangDan
 */

#ifndef RFID_RFID_H_
#define RFID_RFID_H_

#include "../NetworkModule/Common.h"

#define RFID_BUF_LENGTH     2000//1500

typedef struct
{
    bool Flag_RFID_Data_Ready;
    bool Flag_RFID_RX_Begin;
    bool Flag_RFID_RX_Data;
    bool Flag_RFID_RX_End;
    bool Flag_RFID_SD_Store_Done;
    bool Flag_RFID_Data_Waiting;
    bool Flag_RFID_Sending_Inprocess;
    uint16_t RFID_RX_Counter;
    uint16_t RFID_Data_Waiting_Counter;
}RFID_Manager_t;

typedef struct
{
    char RFID_Data_Str[RFID_BUF_LENGTH];
}RFID_Data_t;

void ConfigureUART6_RFID(void);
void Configure_RFID(void);
void UART6_Send(char *cmd);
void RFID_ReceiveData(void);
void RFID_Task_Execute(bool state);

#endif /* RFID_RFID_H_ */
