/*
 * SD.c
 *
 *  Created on: Feb 15, 2022
 *      Author: QuangDan
 */
#include "SD.h"
#include "../MQTT/MQTT_HwLayer.h"
#include "../NetworkModule/GNSS.h"

// ENABLE_LFN should be defined to 1 for using of long file name

#define PATH_BUF_SIZE           80 // Defines the size of the buffer that holds the path
#define CMD_BUF_SIZE            64 // Defines the size of the buffer that holds the command line
#define READ_BUF_SIZE           2000
static char g_pcCwdBuf[PATH_BUF_SIZE] = "/"; // This buffer holds the full path to the current working directory.  Initially it is root ("/").
static char g_pcTmpBuf[PATH_BUF_SIZE];// A temporary data buffer used when manipulating file paths, or reading data from the SD card.
//static char g_dataTmpBuf[1000];
char g_dataTmpBuf[READ_BUF_SIZE + 1];
char g_pcCmdBuf[CMD_BUF_SIZE]; // The buffer that holds the command line.
// The following are data structures used by FatFs.
FATFS g_sFatFs;
static DIR g_sDirObject;
static FILINFO g_sFileInfo;
static FIL g_sFileObject;

char FileName_Tmp[50];
char Time_Buf[50];
SD_FileName_t SD_FileName;

uint16_t Read_Block_Num = 0;

extern GNSS_Manager_t GNSS_Manager;

typedef struct
{
    FRESULT iFResult;
    char *pcResultStr;
}
tFResultString;

SD_Param_t SD_Param;
extern Mqtt_Params_t Mqtt_Params;

// A macro to make it easy to add result codes to the table.
//*****************************************************************************
#define FRESULT_ENTRY(f)        { (f), (#f) }

//------------------------------------------------------------------------------------------------------------------------------------//
tFResultString g_psFResultStrings[] =
{
    FRESULT_ENTRY(FR_OK),
    FRESULT_ENTRY(FR_DISK_ERR),
    FRESULT_ENTRY(FR_INT_ERR),
    FRESULT_ENTRY(FR_NOT_READY),
    FRESULT_ENTRY(FR_NO_FILE),
    FRESULT_ENTRY(FR_NO_PATH),
    FRESULT_ENTRY(FR_INVALID_NAME),
    FRESULT_ENTRY(FR_DENIED),
    FRESULT_ENTRY(FR_EXIST),
    FRESULT_ENTRY(FR_INVALID_OBJECT),
    FRESULT_ENTRY(FR_WRITE_PROTECTED),
    FRESULT_ENTRY(FR_INVALID_DRIVE),
    FRESULT_ENTRY(FR_NOT_ENABLED),
    FRESULT_ENTRY(FR_NO_FILESYSTEM),
    FRESULT_ENTRY(FR_MKFS_ABORTED),
    FRESULT_ENTRY(FR_TIMEOUT),
    FRESULT_ENTRY(FR_LOCKED),
    FRESULT_ENTRY(FR_NOT_ENOUGH_CORE),
    FRESULT_ENTRY(FR_TOO_MANY_OPEN_FILES),
    FRESULT_ENTRY(FR_INVALID_PARAMETER),
};

//*****************************************************************************
//
// A macro that holds the number of result codes.
//
//*****************************************************************************
#define NUM_FRESULT_CODES       (sizeof(g_psFResultStrings) /                 \
                                 sizeof(tFResultString))

//*****************************************************************************

const char *StringFromFResult(FRESULT iFResult)
{
    uint_fast8_t ui8Idx;
    for(ui8Idx = 0; ui8Idx < NUM_FRESULT_CODES; ui8Idx++)
    {

        if(g_psFResultStrings[ui8Idx].iFResult == iFResult)
        {
            return(g_psFResultStrings[ui8Idx].pcResultStr);
        }
    }
    return("UNKNOWN ERROR CODE");
}

//*****************************************************************************
//
// This is the handler for this SysTick interrupt.  FatFs requires a timer tick
// every 10 ms for internal timing purposes.
//
//*****************************************************************************
uint16_t SysTick_Counter = 0;
void SysTickHandler(void)
{
    disk_timerproc();
//    SysTick_Counter++;
//    if(SysTick_Counter >= 100)
//    {
//        SysTick_Counter = 0;
//        UARTprintf("SD systick\r\n");
//    }
}

//*****************************************************************************
int Cmd_ls(int argc, char *argv[])
{
    uint32_t ui32TotalSize;
    uint32_t ui32FileCount;
    uint32_t ui32DirCount;
    FRESULT iFResult;
    FATFS *psFatFs;
    char *pcFileName;
#if _USE_LFN
    char pucLfn[_MAX_LFN + 1];
    g_sFileInfo.lfname = pucLfn;
    g_sFileInfo.lfsize = sizeof(pucLfn);
#endif

    iFResult = f_opendir(&g_sDirObject, g_pcCwdBuf);
    if(iFResult != FR_OK)
    {
        return((int)iFResult);
    }

    ui32TotalSize = 0;
    ui32FileCount = 0;
    ui32DirCount = 0;
    for(;;)
    {

        iFResult = f_readdir(&g_sDirObject, &g_sFileInfo);// Read an entry from the directory.
        if(iFResult != FR_OK)
        {
            return((int)iFResult);
        }
        // If the file name is blank, then this is the end of the listing.
        if(!g_sFileInfo.fname[0])
        {
            break;
        }
        if(g_sFileInfo.fattrib & AM_DIR)// If the attribue is directory, then increment the directory count.
        {
            ui32DirCount++;
        }
        else
        {
            ui32FileCount++;
            ui32TotalSize += g_sFileInfo.fsize;
        }

#if _USE_LFN
        pcFileName = ((*g_sFileInfo.lfname)?g_sFileInfo.lfname:g_sFileInfo.fname);
#else
        pcFileName = g_sFileInfo.fname;
#endif
    }
    iFResult = f_getfree("/", (DWORD *)&ui32TotalSize, &psFatFs);
    if(iFResult != FR_OK)
    {
        return((int)iFResult);
    }
    return(0);
}

//*****************************************************************************
int Cmd_cd(int argc, char *argv[])
{
    uint_fast8_t ui8Idx;
    FRESULT iFResult;
    strcpy(g_pcTmpBuf, g_pcCwdBuf); // Copy the current working path into a temporary buffer so it can be manipulated.

    if(argv[1][0] == '/') // If the first character is /, then this is a fully specified path, and it should just be used as-is.
    {
        if(strlen(argv[1]) + 1 > sizeof(g_pcCwdBuf))
        {
            return(0);
        }
        else
        {
            strncpy(g_pcTmpBuf, argv[1], sizeof(g_pcTmpBuf));
        }
    }
    else if(!strcmp(argv[1], ".."))
    {
        ui8Idx = strlen(g_pcTmpBuf) - 1;
        while((g_pcTmpBuf[ui8Idx] != '/') && (ui8Idx > 1))
        {
            ui8Idx--;
        }
        g_pcTmpBuf[ui8Idx] = 0;
    }
    else
    {
        if(strlen(g_pcTmpBuf) + strlen(argv[1]) + 1 + 1 > sizeof(g_pcCwdBuf))
        {
            return(0);
        }
        else
        {
            if(strcmp(g_pcTmpBuf, "/"))
            {
                strcat(g_pcTmpBuf, "/");
            }
            strcat(g_pcTmpBuf, argv[1]);
        }
    }
    iFResult = f_opendir(&g_sDirObject, g_pcTmpBuf);
    if(iFResult != FR_OK)
    {
        //UARTprintf("cd: %s\n", g_pcTmpBuf);
        return((int)iFResult);
    }
    else
    {
        strncpy(g_pcCwdBuf, g_pcTmpBuf, sizeof(g_pcCwdBuf));
    }
    return(0);
}
//*****************************************************************************
int Cmd_pwd(int argc, char *argv[])
{
    //UARTprintf("%s\n", g_pcCwdBuf);
    return(0);
}
//*****************************************************************************
int Cmd_CreateFile(int argc, char *argv[])
{

    /*
     * Khi rút thẻ nhớ ra, vì một lý do nào đấy khi cắm thẻ nhớ lại thì vđk không cho phép ghi dữ liệu ra thẻ.
     * Do đó cần khởi tạo lại thẻ nhớ như lúc bắt đầu chương trình
     */
    FRESULT iFResult;
    FRESULT fr;
    uint32_t BytesWrite = 0;
    //////////////////////////////////////////////////////////////////////////////// Khởi tạo lại thẻ nhớ.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    // Configure SysTick for a 100Hz interrupt.  The FatFs driver wants a 10 ms tick.
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / 100);
    ROM_SysTickEnable();
    ROM_SysTickIntEnable();
    ROM_IntMasterEnable();

    iFResult = f_mount(0, &g_sFatFs);
    if(iFResult != FR_OK)
    {
        return(1);
    }
    ////////////////////////////////////////////////////////////////////////////////

    memset(g_pcTmpBuf, 0, sizeof(g_pcTmpBuf));
    if(strlen(g_pcCwdBuf) + strlen(argv[1]) + 1 + 1 > sizeof(g_pcTmpBuf))
    {
        return(0);
    }
    strcpy(g_pcTmpBuf, g_pcCwdBuf);
    if(strcmp("/", g_pcCwdBuf))
    {
        strcat(g_pcTmpBuf, "/");
    }
    strcat(g_pcTmpBuf, argv[1]);

    ////////////////////////////////////////////////////////////////////////////////////// Open the file for creating new file.
    iFResult = f_open(&g_sFileObject, g_pcTmpBuf, FA_CREATE_NEW);
    memset(g_pcTmpBuf, 0, sizeof(g_pcTmpBuf));
    if(iFResult != FR_OK)
    {
        return((int)iFResult);
    }
    f_close(&g_sFileObject);
    return(0);
}
//*****************************************************************************
int Cmd_WriteFile(int argc, char *argv[])
{
    FRESULT iFResult;
    FRESULT fr;
    uint32_t BytesWrite = 0;
    memset(g_pcTmpBuf, 0, sizeof(g_pcTmpBuf));

    if(strlen(FileName_Tmp) + strlen(argv[1]) + 1 + 1 > sizeof(g_pcTmpBuf))
    {
        return(0);
    }
    strcpy(g_pcTmpBuf, FileName_Tmp);
    if(strcmp("/", FileName_Tmp))
    {
        strcat(g_pcTmpBuf, "/");
    }
    strcat(g_pcTmpBuf, argv[1]);

    ///////////////////////////////////////////////////////////////////////////////////// Open the file for writing.
    //memset(Time_Buf, 0, sizeof(Time_Buf));
    //sprintf(Time_Buf, "TestFile.txt");
    //sprintf(Time_Buf, "%d:%d:%d %d/%d/20%d,",hr, min, s, dt, mt, yr);

    iFResult = f_open(&g_sFileObject, g_pcTmpBuf, FA_WRITE);
     memset(g_pcTmpBuf, 0, sizeof(g_pcTmpBuf));
    if(iFResult != FR_OK)
    {
        memset(Time_Buf, 0, sizeof(Time_Buf));
        return((int)iFResult);
        ////////////////////////////////////////////////////////////////////////////////////// Open the file for creating new file.
    }
    fr = f_lseek(&g_sFileObject, f_size(&g_sFileObject));
    if(fr != FR_OK)
    {
           return((int)iFResult);
    }

    return(0);
}

int Cmd_cat(int argc, char *argv[])
{
    memset(g_dataTmpBuf, 0, sizeof(g_dataTmpBuf));
    FRESULT iFResult;
    uint32_t ui32BytesRead;

    if(strlen(g_pcCwdBuf) + strlen(argv[1]) + 1 + 1 > sizeof(g_pcTmpBuf))
    {
        //UARTprintf("Resulting path name is too long\n");
        return(0);
    }
    // Copy the current path to the temporary buffer so it can be manipulated.
    strcpy(g_pcTmpBuf, g_pcCwdBuf);
    // If not already at the root level, then append a separator.
    if(strcmp("/", g_pcCwdBuf))
    {
        strcat(g_pcTmpBuf, "/");
    }
    // Now finally, append the file name to result in a fully specified file.
    strcat(g_pcTmpBuf, argv[1]);
    //////////////////////////////////////////////////////////////////////////////////// Open the file for reading.
    iFResult = f_open(&g_sFileObject, g_pcTmpBuf, FA_READ);
    if(iFResult != FR_OK)
    {
        return((int)iFResult);
    }
    UARTprintf("\r\n[SD] Data: \r\n");
    Read_Block_Num = 0;
    do
    {
        if(SD_Param.Flag_SD_Data_Block_Request == true)
        {
            SD_Param.Flag_SD_Data_Block_Request = false;
            // Read a block of data from the file.  Read as much as can fit in the temporary buffer, including a space for the trailing null.
            iFResult = f_read(&g_sFileObject, g_dataTmpBuf, sizeof(g_dataTmpBuf) - 1, (UINT *)&ui32BytesRead);
            if(iFResult != FR_OK)// If there was an error reading, then print a newline and return the error to the user.
            {
                return((int)iFResult);
            }

            g_dataTmpBuf[ui32BytesRead] = 0; //Kí tự kết thúc một string để có thể gửi lên máy tính.
            Read_Block_Num++;

            UARTprintf("%s", g_dataTmpBuf);

            UARTprintf("\r\n[MQTT] Data ready\r\n");

            if(strstr(g_dataTmpBuf, ">"))
            {
                SD_Param.Flag_SD_Data_End = true;
            }

            // Publish data through MQTT
            sprintf(Mqtt_Params.pubTopic_Str, TEST_IMG_TOPIC, DEVICE_ID);
            uint8_t topicLen = strlen(Mqtt_Params.pubTopic_Str);

            char Pub_Data[READ_BUF_SIZE + 20];

            sprintf(Pub_Data, "\"*%04d,%s#\"", Read_Block_Num, g_dataTmpBuf);
            MQTT_PublishData(Mqtt_Params.pubTopic_Str, 1, Pub_Data);
        }
    }
    while(ui32BytesRead == sizeof(g_dataTmpBuf) - 1);
    f_close(&g_sFileObject);
    UARTprintf("\r\n[SD] End\r\n");
    return(0);
}

//*****************************************************************************
int Cmd_help(int argc, char *argv[])
{
    tCmdLineEntry *psEntry;
    psEntry = &g_psCmdTable[0];
    while(psEntry->pcCmd)
    {
        psEntry++;
    }
    return(0);
}
//*****************************************************************************
tCmdLineEntry g_psCmdTable[] =
{
    { "help",   Cmd_help,           "Display list of commands" },
    { "h",      Cmd_help,           "alias for help" },
    { "?",      Cmd_help,           "alias for help" },
    { "ls",     Cmd_ls,             "Display list of files" },
    { "chdir",  Cmd_cd,             "Change directory" },
    { "cd",     Cmd_cd,             "alias for chdir" },
    { "pwd",    Cmd_pwd,            "Show current working directory" },
    { "cat",    Cmd_cat,            "Show contents of a text file" },
    { "create", Cmd_CreateFile,     "Create new file" },
    { "write",  Cmd_WriteFile,      "Write to existing file" },
    { 0, 0, 0 }
};

// The error routine that is called if the driver library encounters an error.
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif
//*****************************************************************************

void SendCommand(char *cmd, char *pcBuf, uint32_t ui32Len)
{
    uint32_t ui32Count = 0;
    int8_t cChar;
    static int8_t bLastWasCR = 0;
    ui32Len--;
    while(*cmd != '\0')
    {
        cChar = *cmd++;
        if(cChar == '\b')
        {
            if(ui32Count)
            {
                UARTwrite("\b \b", 3);
                ui32Count--;
            }
            continue;
        }
        if((cChar == '\n') && bLastWasCR)
        {
            bLastWasCR = 0;
            continue;
        }
        if((cChar == '\r') || (cChar == '\n') || (cChar == 0x1b))
        {
            if(cChar == '\r')
            {
                bLastWasCR = 1;
            }
            break;
        }
        if(ui32Count < ui32Len)
        {
            pcBuf[ui32Count] = cChar;
            ui32Count++;
        }
    }
    pcBuf[ui32Count] = 0;
}
//------------------------------------------------------------------------------------------------------------------------------------//
void SD_Open(char *SD_FilePath)
{
    char FileName[50];
    int nStatus;
    memset(FileName_Tmp, 0, sizeof(FileName_Tmp));

    sprintf(FileName_Tmp, "%s\r", SD_FilePath);
    sprintf(FileName, "write %s", FileName_Tmp);

    SendCommand(FileName, g_pcCmdBuf, sizeof(g_pcCmdBuf));
    nStatus = CmdLineProcess(g_pcCmdBuf);

    if(nStatus == CMDLINE_BAD_CMD)
    {
        UARTprintf("[SD] Bad command!\n");
    }

    memset(FileName, 0, sizeof(FileName));
}
//------------------------------------------------------------------------------------------------------------------------------------//
void SD_Write(void *SD_Data)
{
    uint16_t BytesWrite = 0;
    f_write(&g_sFileObject, SD_Data, strlen(SD_Data),(UINT *)&BytesWrite);
}
//------------------------------------------------------------------------------------------------------------------------------------//
void SD_GNSS_Write(void *SD_Data)
{
    uint16_t BytesWrite = 0;
    if(GNSS_Manager.flag_GNSS_file_created == false)
    {
        GNSS_Manager.flag_GNSS_file_created = true;
        f_write(&g_sFileObject, "Date time,", sizeof("Date time,") - 1,(UINT *)&BytesWrite);
        f_write(&g_sFileObject, "Latitude,", sizeof("Latitude,") - 1,(UINT *)&BytesWrite);
        f_write(&g_sFileObject, "Longitude,", sizeof("Longitude,") - 1,(UINT *)&BytesWrite);
        f_write(&g_sFileObject, "Accuracy (m),", sizeof("Accuracy (m),") - 1,(UINT *)&BytesWrite);
        f_write(&g_sFileObject, "Speed (kph),", sizeof("Speed (kph),") - 1,(UINT *)&BytesWrite);
        f_write(&g_sFileObject, "\r", sizeof("\r") - 1,(UINT *)&BytesWrite); // Xuống hàng
        f_write(&g_sFileObject, "\n", sizeof("\n") - 1,(UINT *)&BytesWrite); // Xuống hàng
    }
    else if(GNSS_Manager.flag_GNSS_file_created == true)
    {
        f_write(&g_sFileObject, SD_Data, strlen(SD_Data),(UINT *)&BytesWrite);
        //f_close(&g_sFileObject);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------//
void SD_Close()
{
    f_close(&g_sFileObject);
}
//------------------------------------------------------------------------------------------------------------------------------------//
void SD_Read(char* SD_FilePath)
{
    char FileName[50];
    int nStatus;
    sprintf(FileName, "cat %s", SD_FilePath);
    SendCommand(FileName, g_pcCmdBuf, sizeof(g_pcCmdBuf));
    nStatus = CmdLineProcess(g_pcCmdBuf);

    if(nStatus == CMDLINE_BAD_CMD)
    {
        UARTprintf("[SD] Bad command!\n");
    }

    memset(FileName, 0, sizeof(FileName));
    //SendCommand(SD_FilePath, g_pcCmdBuf, sizeof(g_pcCmdBuf));
    //CmdLineProcess(g_pcCmdBuf);
}
//------------------------------------------------------------------------------------------------------------------------------------//
void SD_OpenPath(char* SD_Directory)
{
    SendCommand(SD_Directory,g_pcCmdBuf, sizeof(g_pcCmdBuf));
    CmdLineProcess(g_pcCmdBuf);
}
//------------------------------------------------------------------------------------------------------------------------------------//
void SD_DEBUG(void)
{
    int nStatus;
    UARTgets(g_pcCmdBuf, sizeof(g_pcCmdBuf));
    nStatus = CmdLineProcess(g_pcCmdBuf);
    if(nStatus == CMDLINE_BAD_CMD)
    {
        UARTprintf("[SD] Bad command!\n");
    }

    else if(nStatus == CMDLINE_TOO_MANY_ARGS)
    {
        UARTprintf("[SD] Too many arguments for command processor!\n");
    }
}
//------------------------------------------------------------------------------------------------------------------------------------//
void SD_CreateFile_TimeSync(void)
{
    char NewFileName[50];
    int nStatus;
    sprintf(NewFileName, "create %s", FileName_Tmp);
    SendCommand(NewFileName, g_pcCmdBuf, sizeof(g_pcCmdBuf));
    nStatus = CmdLineProcess(g_pcCmdBuf);

    if(nStatus == CMDLINE_BAD_CMD)
    {
        UARTprintf("[SD] Bad command!\n");
    }

    memset(NewFileName, 0, sizeof(NewFileName));
}
//------------------------------------------------------------------------------------------------------------------------------------//
void SD_CreateFile(char* SD_FilePath)
{
    memset(FileName_Tmp, 0, sizeof(FileName_Tmp));
    sprintf(FileName_Tmp, "%s\r", SD_FilePath);
    SD_CreateFile_TimeSync();
}
//------------------------------------------------------------------------------------------------------------------------------------//
//void SD_Init(char* SD_FilePath)
void SD_Init()
{
    FRESULT iFResult;

    SD_Param.Flag_SD_Data_Begin = true;
    SD_Param.Flag_SD_Data_End = false;
    SD_Param.Flag_SD_Data_Block_Request = false;
    SD_Param.Flag_SD_Data_Request = false;
    SD_Param.Flag_SD_Data_Ready = false;

    // Configure SysTick for a 100Hz interrupt.  The FatFs driver wants a 10 ms tick.
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / 100);
    ROM_SysTickEnable();
    ROM_SysTickIntEnable();
    ROM_IntMasterEnable();

    iFResult = f_mount(0, &g_sFatFs);
    if(iFResult != FR_OK)
    {
        UARTprintf("[SD] SD init failure!\n");
    }
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/100);

//    memset(FileName_Tmp, 0, sizeof(FileName_Tmp));
//    //sprintf(FileName_Tmp, "CAMTest3.txt\r");
//    sprintf(FileName_Tmp, "%s\r", SD_FilePath);
//    SD_CreateFile_TimeSync();
}

