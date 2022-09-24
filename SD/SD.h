/*
 * SD.h
 *
 *  Created on: Feb 15, 2022
 *      Author: QuangDan
 */

#ifndef SD_SD_H_
#define SD_SD_H_

#include "../NetworkModule/Common.h"
#include "../utils/cmdline.h"
#include "../NetworkModule/uartstdio.h"
#include "../third_party/fatfs/src/ff.h"
#include "../third_party/fatfs/src/diskio.h"

typedef struct
{
    bool Flag_SD_Data_Ready;
    bool Flag_SD_Data_Begin;
    bool Flag_SD_Data_RX_Data;
    bool Flag_SD_Data_End;
    bool Flag_SD_Data_Request;
    bool Flag_SD_Data_Block_Request;
    bool Flag_SD_GNSS_Store_Request;
}SD_Param_t;

typedef struct
{
    char SD_GNSS_FileName[100];
    char SD_RFID_FileName[100];
}SD_FileName_t;

void SD_CreateFile_TimeSync(void);
void SD_Init();
void SD_Read(char* SD_FilePath);
void SD_OpenPath(char* SD_Directory);
void SD_Open(char *SD_FilePath);
void SD_Close();
void SD_Write(void *SD_Data);
void SD_GNSS_Write(void *SD_Data);
void SD_CreateFile(char* SD_FilePath);

#endif /* SD_SD_H_ */
