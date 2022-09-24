/*
 * GNSS.h
 *
 *  Created on: Jan 19, 2022
 *      Author: QuangDan
 */

#ifndef NETWORKMODULE_GNSS_H_
#define NETWORKMODULE_GNSS_H_

#define GNSS_Buf_Size       200
#define GNSS_Display_All    0

typedef struct
{
    uint8_t pwron_step;
    uint8_t pwroff_step;
    bool    flag_GNSS_fixed;
    bool    flag_GNSS_file_created;
    bool    flag_GNSS_init_done;
    bool    flag_GNSS_pub_done;
    bool    flag_GNSS_sending_inprocess;
    uint32_t GNSS_Timer_Counter;
    uint32_t GNSS_Scan_Counter;
    uint32_t GNSS_Scan_Counter_Prev;
} GNSS_Manager_t;

typedef struct
{
    float GNSS_latitude;
    float GNSS_longitude;
    float GNSS_speed_kph;
    float GNSS_heading;
    float GNSS_altitude;
    uint16_t GNSS_accuracy;
    uint16_t GNSS_year;
    uint8_t GNSS_month;
    uint8_t GNSS_day;
    uint8_t GNSS_hour;
    uint8_t GNSS_minute;
    uint8_t GNSS_sec;
    uint16_t GNSS_Scan_Period;
} GNSS_Params_t;

char GNSS_Buf[GNSS_Buf_Size];

uint8_t GPS_Decode(char *buffer, float *lat, float *lon, float *speed_kph, float *heading, float *altitude, uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *min, uint8_t *sec);
void GPS_Process_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
void TimerA1IntHandler(void);
void ConfigureTimer_A1(uint16_t timer_tick_ms);
void GNSS_Task_Execute(void);
bool GPSPositioning(char *buffer);
bool GNSS_Decode(char *ResponseBuffer);
static void GNSS_PowerOn_Callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
void GNSS_EnableReading(bool status);
long convert_date_to_epoch(int day_t,int month_t,int year_t ,int hour_t,int minute_t,int second_t);
void GNSS_Position_Time_Str_Convert(char *str, uint8_t bat_lev, float accuracy, float lat, float lon, float alt, uint16_t speed, uint16_t year_t, uint8_t month_t, uint8_t date_t, uint8_t hour_t, uint8_t min_t, uint8_t sec_t);

#endif /* NETWORKMODULE_GNSS_H_ */
