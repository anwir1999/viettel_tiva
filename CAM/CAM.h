/*
 * RFID.h
 *
 *  Created on: Feb 14, 2022
 *      Author: QuangDan
 */

#ifndef CAM_CAM_H_
#define CAM_CAM_H_

#include "../NetworkModule/Common.h"

typedef struct
{
    bool Flag_CAM_Data_Ready;
    bool Flag_CAM_RX_Begin;
    bool Flag_CAM_RX_Data;
    bool Flag_CAM_RX_End;
    char CAM_Data_Str[2000];
    bool Flag_CAM_Require;

}CAM_Param_t;

void ConfigureUART7_CAM(void);
void UART7_Send(char *cmd);
void CAM_ReceiveData(void);
void CAM_TakePhoto(void);

#endif /* CAM_CAM_H_ */
