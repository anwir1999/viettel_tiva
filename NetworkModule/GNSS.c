/*
 * GNSS.c
 *
 *  Created on: Jan 19, 2022
 *      Author: QuangDan
 */
#include "Common.h"
#include "AT_Function.h"
#include "GNSS.h"
#include "time.h"
#include "../TFT_LCD_Lib/IFTSPI2_2LCD.h"
#include "../TFT_LCD_Lib/IFT_LCD_PenColor.h"
#include "../TFT_LCD_Lib/ColorTFTSymbols.h"
#include "../MQTT/MQTT_HwLayer.h"
#include "../DS3231.h"
#include "../Cellular/cellular.h"
#include "../SD/SD.h"

extern Mqtt_Params_t Mqtt_Params;
extern DS3231_Time_t DS3231_Time;
extern Device_Flag D_Flag_t;
extern Cell_Manager_t Cell_Manager;
extern Cell_Flag_t Cell_Flag;
extern SD_FileName_t SD_FileName;
extern SD_Param_t   SD_Param;
extern uint8_t MQTT_MsgType;
extern char LCD_Buf[200];
extern LCD_Manager_t LCD_Manager;

GNSS_Manager_t GNSS_Manager;
GNSS_Params_t  GNSS_Params;

//uint8_t GPS_Scan_Counter = 0;
//uint8_t GPS_Fix_Status = 0;

//------------------------------------------------------------------------------------------------------------------------------------//
static void GNSS_PowerOn_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
    if(event == SIMCOM_EVEN_OK)
    {
        switch (GNSS_Manager.pwron_step)
        {
            case 0:
                // Disable main task timer
                IntDisable(INT_TIMER4A);
                Simcom_ATC_SendATCommand("AT+CGNSSPWR?\r\n", "+CGNSSPWR:", 2000, 1, GNSS_PowerOn_Callback);
                GNSS_Manager.pwron_step++;
                break;
            case 1:
                if(strstr(ResponseBuffer, "+CGNSSPWR: 1"))
                {
                    GNSS_PowerOn_Callback(SIMCOM_EVEN_OK, "");
                }
                if(strstr(ResponseBuffer, "+CGNSSPWR: 0"))
                {
                    Simcom_ATC_SendATCommand("AT+CGNSSPWR=1\r\n", "+CGNSSPWR: READY!", 30000, 0, GNSS_PowerOn_Callback);
                    GNSS_Manager.pwron_step++;
                }
                break;
            case 2:
                Simcom_ATC_SendATCommand("AT+CGNSSMODE=3\r\n", "OK", 5000, 5, GNSS_PowerOn_Callback);
                GNSS_Manager.pwron_step++;
                break;
            case 3:
                GNSS_Manager.flag_GNSS_init_done = true;
                // Enable main task timer
                IntEnable(INT_TIMER4A);
                ConfigureTimer_A1(1);
                D_Flag_t.Flag_GPS_Started = true;
                GNSS_Manager.pwron_step++;
                break;
            default:
                break;
        }
        //GNSS_Manager.pwron_step++;
    }
    if(event == SIMCOM_EVEN_TIMEOUT)
    {
        UARTprintf("[GNSS] Power on GNSS: SIMCOM_EVENT_TIMEOUT");
        GNSS_Manager.pwron_step = 0;
        GNSS_Manager.flag_GNSS_fixed = false;
        GNSS_Manager.flag_GNSS_init_done = false;
        D_Flag_t.Flag_NeedToRebootNetworkModule = true;
        SysCtlReset();
    }
    if(event == SIMCOM_EVEN_ERROR)
    {
        UARTprintf("[GNSS] Power on GNSS: SIMCOM_EVENT_ERROR");
        GNSS_Manager.pwron_step = 0;
        GNSS_Manager.flag_GNSS_fixed = false;
        GNSS_Manager.flag_GNSS_init_done = false;
        D_Flag_t.Flag_NeedToRebootNetworkModule = true;
        SysCtlReset();
    }
}
void GNSS_Task_Execute(void)
{

    if(GNSS_Manager.flag_GNSS_init_done == false)
    {
        // Newly added
        MQTT_MsgType = FL_MQTT_GNSS;
        GNSS_Manager.pwron_step = 0;
        GNSS_Manager.flag_GNSS_fixed = false;
        GNSS_Params.GNSS_Scan_Period = 1;
        GNSS_PowerOn_Callback(SIMCOM_EVEN_OK, "");
        //Simcom_ATC_SendATCommand("AT+CGNSSPWR=0\r\n", "OK", 1000, 2, GNSS_PowerOn_Callback);
    }
    else
    {
        // Newly added
        MQTT_MsgType = FL_MQTT_GNSS;
        GNSS_Manager.flag_GNSS_fixed = false;
        GNSS_EnableReading(true);
    }
}
void ConfigureTimer_A1(uint16_t timer_tick_ms)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, (SysCtlClockGet() / timer_tick_ms) -1);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();
    TimerEnable(TIMER1_BASE, TIMER_A);
}

void TimerA1IntHandler(void)
{
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    GNSS_Params.GNSS_Scan_Period = 1;
    // Increase GNSS_Timer_Counter when finishing getting BTS cell information
    if(Cell_Manager.cellState == CELL_IDLE)
    {
        GNSS_Manager.GNSS_Timer_Counter++;
    }
    // Process GNSS
    if( GNSS_Manager.GNSS_Timer_Counter >= GNSS_Params.GNSS_Scan_Period && GNSS_Manager.flag_GNSS_fixed == false)
    {
        GNSS_Manager.GNSS_Timer_Counter = 0;
        if(GNSS_Manager.flag_GNSS_init_done == true)
        {
            GNSS_Manager.GNSS_Scan_Counter++;
            UARTprintf("[GNSS] GNSS scan time: %d s\r\n", GNSS_Manager.GNSS_Scan_Counter);

            // Get time and date
            DS3231_Get_Time_Date();
            UARTprintf("[DS3231] Time: %02d/%02d/20%02d %02d:%02d:%02d\r\n",DS3231_Time.dt, DS3231_Time.mt, DS3231_Time.yr, DS3231_Time.hr, DS3231_Time.min, DS3231_Time.s);

            // Get GNSS information
            TimerDisable(TIMER1_BASE, TIMER_A);
            Simcom_ATC_SendATCommand("AT+CGNSSINFO\r\n", "+CGNSSINFO:", 1000, 1, GPS_Process_Callback);

            // GNSS scan timeout
            if(GNSS_Manager.GNSS_Scan_Counter >= 1000)
            {
                GNSS_Manager.GNSS_Scan_Counter = 0;
            }

            // GNSS scan timeout
//            if(GNSS_Manager.GNSS_Scan_Counter >= 300)
//            {
//                SysCtlReset();
//                GNSS_EnableReading(false);
//                if(AT_RX_event == SIMCOM_EVEN_OK)
//                {
//                    Simcom_ATC_SendATCommand("AT+CGNSSPWR=0\r\n", "OK", 1000, 2, Simcom_ATResponse_Callback);
//                    GNSS_Manager.GNSS_Scan_Counter = 0;
//                }
//            }
        }
    }
    if(GNSS_Manager.flag_GNSS_fixed == true)
    {
        //GNSS_Manager.flag_GNSS_pub_done = false;
        GNSS_Manager.GNSS_Scan_Counter = 0;
        GNSS_Manager.flag_GNSS_sending_inprocess = true;
        //GNSS_Params.GNSS_Scan_Period = 1;
        // Get BTS cell information before send the MQTT messages
        if(Cell_Flag.Flag_Cell_GetBTInfor_Done == false)
        {
            Cellular_SwitchState(CELL_ONLYBTSINFOR);
            // Disable cellular task
            TimerDisable(TIMER3_BASE, TIMER_A);
            SD_Param.Flag_SD_GNSS_Store_Request = true;
        }

        // Wait for finishing getting BTS cell information
        if(Cell_Manager.cellState == CELL_IDLE)
        {
            // Disable GNSS timer while sending MQTT messages
            GNSS_EnableReading(false);
            GNSS_Manager.flag_GNSS_fixed = false;
            Cell_Flag.Flag_Cell_GetBTInfor_Done = false;

            // Publish data through MQTT
            memset(Mqtt_Params.pubTopic_Str, 0, sizeof(Mqtt_Params.pubTopic_Str));
            sprintf(Mqtt_Params.pubTopic_Str, INNOWAY_TOPIC_FLEET, DEVICE_ID);
            char Pub_Data[500];
            GNSS_Position_Time_Str_Convert(Pub_Data, 100, GNSS_Params.GNSS_accuracy, GNSS_Params.GNSS_latitude, GNSS_Params.GNSS_longitude, GNSS_Params.GNSS_altitude, GNSS_Params.GNSS_speed_kph, GNSS_Params.GNSS_year, GNSS_Params.GNSS_month, GNSS_Params.GNSS_day, GNSS_Params.GNSS_hour, GNSS_Params.GNSS_minute, GNSS_Params.GNSS_sec);
            MQTT_PublishData(Mqtt_Params.pubTopic_Str, 1, Pub_Data);
        }
    }
}
//-----------------------------------------------------------------------------------------// GPS process callback
void GPS_Process_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer)
{
    AT_RX_event = event;
    if(event == SIMCOM_EVEN_OK)
    {
        TimerEnable(TIMER1_BASE, TIMER_A);

        // Process GNSS data
        if(strstr(GNSS_Buf, "+CGNSSINFO:"))
        {
            GNSS_Decode(GNSS_Buf);
        }
    }
    else if(event == SIMCOM_EVEN_TIMEOUT)
    {
        UARTprintf("[GNSS] SIMCOM_EVENT_TIMEOUT\r\n");
        TimerEnable(TIMER1_BASE, TIMER_A);
        SysCtlReset();
    }

    else if(event == SIMCOM_EVEN_ERROR)
    {
        UARTprintf("[GNSS] SIMCOM_EVENT_ERROR\r\n");
        TimerEnable(TIMER1_BASE, TIMER_A);
        SysCtlReset();
    }
}
//-----------------------------------------------------------------------------------------// GPS decode
bool GNSS_Decode(char *ResponseBuffer)
{
    char LatDD[3], LatMM[10], LogDD[4], LogMM[10], DdMmYy[7] , UTCTime[7];
    char Lat_Str[15], Speed_Str[10], Long_Str[15], Accuracy_Str[10];
    float Speed_f = 0;

    memset(LatDD, '\0', sizeof(LatDD)); // Initialize the string
    memset(LatMM, '\0', sizeof(LatMM)); // Initialize the string
    memset(LogDD, '\0', sizeof(LogDD)); // Initialize the string
    memset(LogMM, '\0', sizeof(LogMM)); // Initialize the string
    memset(DdMmYy, '\0', sizeof(DdMmYy)); // Initialize the string
    memset(UTCTime, '\0', sizeof(UTCTime)); // Initialize the string
    memset(Speed_Str, '\0', sizeof(Speed_Str)); // Initialize the string
    memset(Lat_Str, '\0', sizeof(Lat_Str)); // Initialize the string
    memset(Long_Str, '\0', sizeof(Long_Str)); // Initialize the string
    memset(Accuracy_Str, '\0', sizeof(Accuracy_Str)); // Initialize the string

    char GNSS_Buf_Tok[20][20];

    if (strstr(ResponseBuffer, ",,,,,,,,") != NULL || strlen(ResponseBuffer) < strlen("+CGNSSINFO: ,,,,,,,,"))
    {
        GNSS_Manager.flag_GNSS_fixed = false;
        return false;
    }
    else
    {
        LCD_Manager.Flag_LCD_Busy = true;
        LCD_Fill(0,40,319,140, BLACK);
        POINT_COLOR=YELLOW;

        // Lat processing
        filter_comma(ResponseBuffer, 5, 6, GNSS_Buf_Tok[2]);
        //UARTprintf("%s\n", GNSS_Buf_Tok[2]);

        strncpy(LatDD, GNSS_Buf_Tok[2], 2);
        LatDD[2] = '\0';
        strncpy(LatMM, GNSS_Buf_Tok[2] + 2, 9);
        LatMM[9] = '\0';

        GNSS_Params.GNSS_latitude = atoi(LatDD) + (atof(LatMM) / 60);
        sprintf(Lat_Str,"%.5f",GNSS_Params.GNSS_latitude);
        LCD_ShowString(0, 40, "Latitude");
        memset(LCD_Buf, 0, sizeof(LCD_Buf));sprintf(LCD_Buf, "%s", Lat_Str);LCD_ShowString(160, 40, LCD_Buf);
        //UARTprintf("Lat: %s\n", Lat_Str);

        // Long processing
        filter_comma(ResponseBuffer, 7, 8, GNSS_Buf_Tok[4]);
        //UARTprintf("%s\n", GNSS_Buf_Tok[4]);

        strncpy(LogDD, GNSS_Buf_Tok[4], 3);
        LogDD[3] = '\0';
        strncpy(LogMM, GNSS_Buf_Tok[4] + 3, 9);
        LogMM[9] = '\0';

        GNSS_Params.GNSS_longitude = atoi(LogDD) + (atof(LogMM) / 60);
        sprintf(Long_Str,"%.5f",GNSS_Params.GNSS_longitude);
        LCD_ShowString(0, 60, "Longitude");
        memset(LCD_Buf, 0, sizeof(LCD_Buf));sprintf(LCD_Buf, "%s", Long_Str);LCD_ShowString(160, 60, LCD_Buf);
        //UARTprintf("Long: %s\n", Long_Str);
        // Speed processing
        filter_comma(ResponseBuffer, 12, 13, GNSS_Buf_Tok[9]);
        //UARTprintf("%s\n", GNSS_Buf_Tok[9]);

        Speed_f = 0.51444 * atof(GNSS_Buf_Tok[9]);
        GNSS_Params.GNSS_speed_kph = 3.6 * Speed_f;
        sprintf(Speed_Str,"%.3f",Speed_f);
        LCD_ShowString(0, 80, "Speed");
        memset(LCD_Buf, 0, sizeof(LCD_Buf));sprintf(LCD_Buf, "%s m/s", Speed_Str);LCD_ShowString(160, 80, LCD_Buf);
        //UARTprintf("Speed: %s m/s\n", Speed_Str);

        // Accuracy processing
        filter_comma(ResponseBuffer, 14, 15, GNSS_Buf_Tok[10]);
        if(GNSS_Buf_Tok[10] == NULL)
        {
            GNSS_Manager.flag_GNSS_fixed = false;
            return false;
        }
        if(GNSS_Buf_Tok[10][2] == '.')
        {
            GNSS_Buf_Tok[10][2] = '\0';
        }
        GNSS_Params.GNSS_accuracy = atoi(GNSS_Buf_Tok[10]);
        LCD_ShowString(0, 100, "Accuracy");
        memset(LCD_Buf, 0, sizeof(LCD_Buf));sprintf(LCD_Buf, "%d m", GNSS_Params.GNSS_accuracy);LCD_ShowString(160, 100, LCD_Buf);

        // Time
        filter_comma(ResponseBuffer, 10, 11, GNSS_Buf_Tok[7]);
        //UARTprintf("%s\n", GNSS_Buf_Tok[7]);

        // Seconds
        char *ptr = GNSS_Buf_Tok[7] + 4;
        GNSS_Buf_Tok[7][6] = '\0';
        GNSS_Params.GNSS_sec = atof(ptr);
        //UARTprintf("Second: %d \r\n", GNSS_sec);

        // Minutes
        ptr[0] = 0;
        ptr = GNSS_Buf_Tok[7] + 2;
        GNSS_Params.GNSS_minute = atoi(ptr);
        //UARTprintf("Minute: %d \r\n", GNSS_minute);

        // Hours
        ptr[0] = 0;
        ptr = GNSS_Buf_Tok[7] + 0;
        GNSS_Params.GNSS_hour = atoi(ptr);
        //UARTprintf("Hour: %d \r\n", GNSS_hour);

        // Date
        filter_comma(ResponseBuffer, 9, 10, GNSS_Buf_Tok[6]);
        //UARTprintf("%s\n", GNSS_Buf_Tok[6]);

        // Year
        ptr[0] = 0;
        ptr = GNSS_Buf_Tok[6] + 4;
        GNSS_Params.GNSS_year = 2000 + atoi(ptr);
        //UARTprintf("Year: %d \r\n", GNSS_year);

        // Month
        ptr[0] = 0;
        ptr = GNSS_Buf_Tok[6] + 2;
        GNSS_Params.GNSS_month = atoi(ptr);
        //UARTprintf("Month: %d \r\n", GNSS_month);

        // Day
        ptr[0] = 0;
        ptr = GNSS_Buf_Tok[6] + 0;
        GNSS_Params.GNSS_day = atoi(ptr);
        //UARTprintf("Day: %d \r\n", GNSS_day);

        if(GNSS_Buf_Tok[6] == NULL)
        {
           GNSS_Manager.flag_GNSS_fixed = false;
           return false;
        }

        DS3231_Get_Time_Date();
        if(GNSS_Params.GNSS_year != DS3231_Time.yr || GNSS_Params.GNSS_month != DS3231_Time.mt || GNSS_Params.GNSS_day != DS3231_Time.dt || (GNSS_Params.GNSS_hour +7) != DS3231_Time.hr || abs(GNSS_Params.GNSS_minute - DS3231_Time.min) > 2)
        {
            setDate(1, GNSS_Params.GNSS_day, GNSS_Params.GNSS_month, GNSS_Params.GNSS_year - 2000);
            setTime(GNSS_Params.GNSS_hour + 7, GNSS_Params.GNSS_minute, GNSS_Params.GNSS_sec, am, _24_hour_format);
            DS3231_Get_Time_Date();
            UARTprintf("[DS3231] Time: %02d/%02d/20%02d %02d:%02d:%02d\r\n",DS3231_Time.dt, DS3231_Time.mt, DS3231_Time.yr, DS3231_Time.hr, DS3231_Time.min, DS3231_Time.s);
        }

        GNSS_Manager.flag_GNSS_fixed = true;
        LCD_Manager.Flag_LCD_Busy = false;

        return true;
    }
}
//-----------------------------------------------------------------------------------------//
void GNSS_EnableReading(bool status)
{
    GNSS_Manager.flag_GNSS_fixed = false;
    //GNSS_Manager.flag_GNSS_pub_done = true;
    if(status == true)
    {
        TimerEnable(TIMER1_BASE, TIMER_A);
        GNSS_Manager.flag_GNSS_sending_inprocess = false;
        //GPS_Scan_Counter = 0;
        //GNSS_Manager.flag_GNSS_pub_done = true;
    }
    else
    {
        TimerDisable(TIMER1_BASE, TIMER_A);
        //GPS_Scan_Counter = 0;
    }
}
//------------------------------------------------------------------------------------------------------------------------//
void GNSS_Position_Time_Str_Convert(char *str, uint8_t bat_lev, float accuracy, float lat, float lon, float alt, uint16_t speed, uint16_t year_t, uint8_t month_t, uint8_t date_t, uint8_t hour_t, uint8_t min_t, uint8_t sec_t)
{
    UARTprintf("Time: %02d:%02d:%02d %02d/%02d/%04d \r\n", hour_t, min_t, sec_t, date_t, month_t, year_t);
    int interLat, fracLat, interLon, fracLon, interAlt, fracAlt;
    interLat = (int)lat;
    fracLat = (lat - interLat)*1000000;
    interLon = (int)lon;
    fracLon = (lon - interLon)*1000000;
    interAlt = (int)alt;
    fracAlt = (lon - interAlt)*1000000;

    //sync time esp with gps time
    long TS = convert_date_to_epoch(date_t, month_t, year_t, hour_t, min_t, sec_t);
//  sprintf(str, "{\"Type\":\"DPOS\",\"V\":\"%s\",\"ss\":%d,\"r\":%d,\"B\":%d,\"Cn\":\"%s\",\"Acc\":%f,\"lat\":%d.%06d,\"lon\":%d.%06d,\"T\":%ld}", VTAG_Vesion, VTAG_NetworkSignal.RSRP, VTAG_NetworkSignal.RSRQ, bat_lev, Network_Type_Str, accuracy, interLat, fracLat, interLon, fracLon, TS);

    //Use time from esp32 instead of gps time
    //GetDeviceTimestamp();
    sprintf(str, "{\"type\":\"DPOS\",\"ver\":\"%s\",\"ss\":%d,\"r\":%d,\"b\":%d,\"cn\":\"%s\",\"acc\":%d,\"lat\":%d.%06d,\"lon\":%d.%06d,\"v\":%d,\"ts\":%d}", "F1.0.0T", Cell_Manager.RSRP - 140, -19 + (int)(Cell_Manager.RSRQ/2), bat_lev, "LTE", (int) (accuracy), interLat, fracLat, interLon, fracLon, speed, TS);
}
//------------------------------------------------------------------------------------------------------------------------//
long convert_date_to_epoch(int day_t,int month_t,int year_t ,int hour_t,int minute_t,int second_t)
{
    struct tm time;
    time.tm_year    = year_t - 1900;            // Year - 1900
    time.tm_mon     = month_t - 1;              // Month, where 0 = jan
    time.tm_mday    = day_t;                     // Day of the month
    time.tm_hour    = hour_t;
    time.tm_min     = minute_t;
    time.tm_sec     = second_t;
    time.tm_isdst   = -1;                   // Is DST on? 1 = yes, 0 = no, -1 = unknown

    long epoch = (long) __mktime64(&time) - 6*3600;
    //char str[100];
    //sprintf(str, "%lu", epoch);
    UARTprintf("epoch_convert = %d\r\n", epoch);
    //UARTprintf("epoch_convert = %s\r\n", str);
    return epoch;
}
