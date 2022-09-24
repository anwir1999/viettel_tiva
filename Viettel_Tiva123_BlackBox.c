#include "inc/hw_gpio.h" //for macro declaration of GPIO_O_LOCK and GPIO_O_CR
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "driverlib/pwm.h"
#include "string.h"
#include "driverlib/ssi.h"
#include "driverlib/watchdog.h"

#include "grlib/grlib.h"
#include "TFT_LCD_Lib/IFTSPI2_2LCD.h"
#include "TFT_LCD_Lib/IFT_LCD_PenColor.h"
#include "TFT_LCD_Lib/ColorTFTSymbols.h"

#include "DS3231.h"
#include "NetworkModule/uartstdio.h"
#include "NetworkModule/Common.h"
#include "NetworkModule/AT_Function.h"
#include "NetworkModule/GNSS.h"
#include "MQTT/MQTT_HwLayer.h"
#include "Cellular/cellular.h"
#include "CAM/CAM.h"
#include "SD/SD.h"
#include "RFID/RFID.h"
///////////////////////////////////////////////////////////////////////////
#define SamplingRate					100
#define IIR_num_of_SOSs_default 		40
#define DS3231_Time_Sync				0
#define IMG_TEST                        0
/////////////////////////////////////////////////////////////////////////// Khai báo nguyên m?u hàm
void ConfigureTimer_A4(uint16_t timer_tick_ms);
void ConfigureTimer_A1(uint16_t timer_tick_ms);
void ConfigureUART(void);
void ConfigureADC(void);
void ConfigurePWM_Stellaris(void);
void Configure_WatchDogTimer(void);

uint16_t GetADCValue(void);
void PWM_Para_Setup(float freq, float pulsewidth);
void ConfigureTivaPWM(void);

void GPIO_Unlock_Pins(void);
void EC_Meter_Activate(uint8_t command);
void DS3231_Get_Time_Date(void);

uint32_t CRC_Update(uint32_t crc, const void *data, int data_len);

/////////////////////////////////////////////////////////////////////////// SD
extern char FileName_Tmp[50];
extern DS3231_Time_t DS3231_Time;
//LCD
extern char LCD_Buf[200];
extern LCD_Manager_t LCD_Manager;

tContext g_sContext;
tRectangle sRect;

#define		LongSBar		1
#define		ShortSBar		2
unsigned int BACK_COLOR, POINT_COLOR;   //Background color, brush color
int kli=0;

uint16_t Main_Task_Counter = 0;
uint16_t Data_Transmit_Period_Counter = 0;
uint16_t WDT_Counter = 0;

extern Device_Flag D_Flag_t;
extern SIMCOM_ResponseEvent_t AT_RX_event;

// SD
char SD_FilePath[50] = "21_02_2022_19_18_52.txt";
uint32_t crc = 0x1d0f;
extern SD_Param_t SD_Param;
extern SD_FileName_t SD_FileName;
// CAM
extern CAM_Param_t CAM_Param;
// GNSS
extern GNSS_Params_t  GNSS_Params;
extern GNSS_Manager_t GNSS_Manager;
// MQTT
extern uint8_t MQTT_MsgType;
extern Mqtt_Manager_t Mqtt_Manager;
// RFID
extern RFID_Manager_t RFID_Manager;
extern RFID_Data_t RFID_Data;
/**
 * Static table used for the table_driven implementation.
 */

volatile bool g_bFeedWatchdog = true;

static const uint32_t crc_table[16] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};

int main(void)
{
	FPULazyStackingEnable();

	SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_2_5);

	Configure_WatchDogTimer();

	// Unlock some pins
	GPIO_Unlock_Pins();
	// Init UART for debug
	ConfigureUART();

	UARTprintf("Start up device!\r\n");

	// Init LCD
	TivaInit();
	Lcd_Init();

	// Init real time clock
	DS3231_init();
    #if DS3231_Time_Sync
    {
        setDate(1, 26, 02, 22);
        setTime(13, 50, 00, am, _24_hour_format);
    }
    #endif

    POINT_COLOR=GREEN;
    LCD_ShowString(70, 0, "Fleet Management Console");

    // Init SD storage
    SD_Init();

    // Init UARTs
    Configure_AT_Command();
    ConfigureUART7_CAM();
    Configure_RFID();

    // Slight delay
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/10 - 1);

    //UART7_Send("FL,CAM,1\r\n");

    UART6_Send("From RFID port\r\n");

    // Slight delay
	ROM_SysCtlDelay(ROM_SysCtlClockGet()/10 - 1);

	// Power on A7672S
	AT_LOOP:
	// Power key
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/1 - 1);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/1 - 1);

    // Check AT response
    D_Flag_t.Flag_Wait_Exit = false;
    Simcom_ATC_SendATCommand("AT\r\n", "OK", 500, 4, Simcom_ATResponse_Callback);
    WaitandExitLoop(&D_Flag_t.Flag_Wait_Exit);
    if(AT_RX_event == SIMCOM_EVEN_TIMEOUT || AT_RX_event == SIMCOM_EVEN_ERROR)
    {
        goto AT_LOOP;
    }

    D_Flag_t.Flag_Wait_Exit = false;
    Simcom_ATC_SendATCommand("AT+IPREX=921600\r\n", "OK", 500, 4, Simcom_ATResponse_Callback);
    WaitandExitLoop(&D_Flag_t.Flag_Wait_Exit);

    //A7672S_RestartModule();

    LCD_Fill(0,20,319,40, BLACK);
    POINT_COLOR=YELLOW;
    LCD_ShowString(80, 20, "Device is ready to use");
    UARTprintf("Device is ready!\r\n");

    Mqtt_Manager.flag_mqtt_connection_ready = false;
    RFID_Manager.Flag_RFID_Data_Waiting = false;
    GNSS_Manager.flag_GNSS_pub_done = false;
    D_Flag_t.Flag_NeedToRebootNetworkModule = false;

    //g_bFeedWatchdog = false;

    ConfigureTimer_A4(100);

    #if (IMG_TEST == 1)
    {
        MQTT_MsgType = FL_MQTT_IMG;
        DS3231_Get_Time_Date();
        UARTprintf("[DS3231] Time: %02d/%02d/20%02d %02d:%02d:%02d\r\n",DS3231_Time.dt, DS3231_Time.mt, DS3231_Time.yr, DS3231_Time.hr, DS3231_Time.min, DS3231_Time.s);
        sprintf(SD_FilePath, "Image/IMG_%02d_%02d_20%02d_%02d_%02d_%02d.txt", DS3231_Time.dt,  DS3231_Time.mt,  DS3231_Time.yr,  DS3231_Time.hr,  DS3231_Time.min,  DS3231_Time.s);
        SD_CreateFile(SD_FilePath);

        // Disable AT UART
        UARTDisable(UART2_BASE);
        // Start taking photo
        UART7_Send("FL,CAM,1\r\n");
    }
    #else
    {
        //MQTT_MsgType = FL_MQTT_GNSS;
        D_Flag_t.Task_Selection = FL_TASK_RFID;
        // Init cellular module
        Cellular_InitModule();
    }
    #endif

	while(1)
	{
	    // Reboot network module
	    if(D_Flag_t.Flag_NeedToRebootNetworkModule == true)
	    {
	        TimerDisable(TIMER4_BASE, TIMER_A);
	        TimerDisable(TIMER0_BASE, TIMER_A);

	        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
            ROM_SysCtlDelay(ROM_SysCtlClockGet()/1 - 1);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
            ROM_SysCtlDelay(ROM_SysCtlClockGet()/1 - 1);

            TimerEnable(TIMER0_BASE, TIMER_A);

            D_Flag_t.Flag_Wait_Exit = false;
            Simcom_ATC_SendATCommand("AT\r\n", "OK", 500, 4, Simcom_ATResponse_Callback);
            WaitandExitLoop(&D_Flag_t.Flag_Wait_Exit);

            if(AT_RX_event == SIMCOM_EVEN_TIMEOUT || AT_RX_event == SIMCOM_EVEN_ERROR)
            {
                D_Flag_t.Flag_NeedToRebootNetworkModule = true;
                TimerEnable(TIMER4_BASE, TIMER_A);
            }
            else if(AT_RX_event == SIMCOM_EVEN_OK)
            {
                RFID_Manager.Flag_RFID_Data_Waiting = false;
                GNSS_Manager.flag_GNSS_pub_done = false;
                D_Flag_t.Flag_NeedToRebootNetworkModule = false;
                GNSS_Manager.flag_GNSS_init_done = false;

                TimerEnable(TIMER4_BASE, TIMER_A);

                #if (IMG_TEST == 1)
                {
                    MQTT_MsgType = FL_MQTT_IMG;
                    DS3231_Get_Time_Date();
                    UARTprintf("[DS3231] Time: %02d/%02d/20%02d %02d:%02d:%02d\r\n",DS3231_Time.dt, DS3231_Time.mt, DS3231_Time.yr, DS3231_Time.hr, DS3231_Time.min, DS3231_Time.s);
                    sprintf(SD_FilePath, "Image/IMG_%02d_%02d_20%02d_%02d_%02d_%02d.txt", DS3231_Time.dt,  DS3231_Time.mt,  DS3231_Time.yr,  DS3231_Time.hr,  DS3231_Time.min,  DS3231_Time.s);
                    SD_CreateFile(SD_FilePath);

                    // Disable AT UART
                    UARTDisable(UART2_BASE);
                    // Start taking photo
                    UART7_Send("FL,CAM,1\r\n");
                }
                #else
                {
                    //MQTT_MsgType = FL_MQTT_GNSS;
                    D_Flag_t.Task_Selection = FL_TASK_RFID;
                    // Init cellular module
                    Cellular_InitModule();
                }
                #endif
            }
	    }
	    // Store image to SD card
	    if(CAM_Param.Flag_CAM_Data_Ready == true)
	    {
	        CAM_Param.Flag_CAM_Data_Ready = false;
            SD_Open(SD_FilePath);
            SD_Write(CAM_Param.CAM_Data_Str);
            SD_Close();
            memset(CAM_Param.CAM_Data_Str, 0, sizeof(CAM_Param.CAM_Data_Str));
            UART7_Send("FL,ACK,1\r\n");
	    }
	    if(CAM_Param.Flag_CAM_RX_End == true && CAM_Param.Flag_CAM_Data_Ready == false)
	    {
	        CAM_Param.Flag_CAM_RX_End = false;
	        SD_Open(SD_FilePath);
            SD_Write(">");
            SD_Close();

            // Disable AT UART
            UARTEnable(UART2_BASE);

            SD_Param.Flag_SD_Data_Begin = true;
            SD_Param.Flag_SD_Data_End = false;
            Sub_Callback(SIMCOM_EVEN_OK, "+CMQTTSUB:");

            // Init cellular module
            //Cellular_InitModule();
	    }
	    // Read image from SD card
	    if(SD_Param.Flag_SD_Data_Request == true)
	    {
	        UARTprintf("[SD] SD card read\r\n");
	        SD_Param.Flag_SD_Data_Request = false;
	        SD_Open(SD_FilePath);
            SD_Read(SD_FilePath);
            SD_Close();

            D_Flag_t.Task_Selection = FL_TASK_RFID;
            MQTT_MsgType = FL_MQTT_RFID;
            // Enable AT UART
            UARTEnable(UART2_BASE);
            // Enable main task timer and GNSS timer
            IntEnable(INT_TIMER4A);
            IntEnable(INT_TIMER1A);
	    }
	    // Store GNSS data to SD card
	    if(SD_Param.Flag_SD_GNSS_Store_Request == true)
	    {
	        SD_Param.Flag_SD_GNSS_Store_Request = false;

            // Get time and date
            DS3231_Get_Time_Date();
            UARTprintf("[DS3231] Time: %02d/%02d/20%02d %02d:%02d:%02d\r\n",DS3231_Time.dt, DS3231_Time.mt, DS3231_Time.yr, DS3231_Time.hr, DS3231_Time.min, DS3231_Time.s);

            // Write data to SD card
            char sd_data_to_write[250] = "";
            sprintf(sd_data_to_write, "=\"%02d/%02d/20%02d %02d:%02d:%02d\",=\"%.05f\",=\"%.05f\",=\"%d\",=\"%.01f\"\r\n", DS3231_Time.dt, DS3231_Time.mt, DS3231_Time.yr, DS3231_Time.hr, DS3231_Time.min, DS3231_Time.s, GNSS_Params.GNSS_latitude, GNSS_Params.GNSS_longitude, GNSS_Params.GNSS_accuracy, GNSS_Params.GNSS_speed_kph);
            SD_Open(SD_FileName.SD_GNSS_FileName);
            SD_GNSS_Write(sd_data_to_write);
            SD_Close();

            //GPIOPinWrite(LCD_LED_BASE, LCD_LED_PIN, 0);

            // Enable cellular task
            TimerEnable(TIMER3_BASE, TIMER_A);
	    }
	    // Store RFID data to SD card
	    if(RFID_Manager.Flag_RFID_Data_Ready == true)
	    {
	        RFID_Manager.Flag_RFID_Sending_Inprocess = true;
	        RFID_Manager.Flag_RFID_SD_Store_Done = false;
	        RFID_Manager.Flag_RFID_Data_Ready = false;
	        RFID_Manager.Flag_RFID_RX_End = false;
	        RFID_Manager.Flag_RFID_Data_Waiting = false;

	        SD_Open(SD_FileName.SD_RFID_FileName);
            SD_Write(RFID_Data.RFID_Data_Str);
            SD_Close();
            LCD_Manager.Flag_LCD_Busy = true;
            LCD_Fill(0,40,319,60, BLACK);
            POINT_COLOR=YELLOW;
            memset(LCD_Buf, 0, sizeof(LCD_Buf));sprintf(LCD_Buf, "[RFID] Data is received successfully");LCD_ShowString(0, 40, LCD_Buf);
            LCD_Manager.Flag_LCD_Busy = false;

            RFID_Manager.Flag_RFID_SD_Store_Done = true;
	    }
	    // Display time and GNSS scan time
	    if(LCD_Manager.Flag_LCD_Busy == false)
	    {
            if( DS3231_Time.s != DS3231_Time.old_s)
            {
                POINT_COLOR=RED;
                memset(LCD_Buf, 0, sizeof(LCD_Buf));sprintf(LCD_Buf, "%02d:%02d:%02d %02d/%02d/20%02d", DS3231_Time.hr, DS3231_Time.min, DS3231_Time.s, DS3231_Time.dt, DS3231_Time.mt, DS3231_Time.yr);
                LCD_ShowString(80, 220, LCD_Buf);
                DS3231_Time.old_s = DS3231_Time.s;

                // Display GNSS counter to LCD
                if(GNSS_Manager.GNSS_Scan_Counter != GNSS_Manager.GNSS_Scan_Counter_Prev)
                {
                    LCD_Manager.Flag_LCD_Busy = true;
                    LCD_Fill(80,20,319,40, BLACK);
                    POINT_COLOR=YELLOW;
                    memset(LCD_Buf, 0, sizeof(LCD_Buf));sprintf(LCD_Buf, "GPS scan time: %d s", GNSS_Params.GNSS_Scan_Period * GNSS_Manager.GNSS_Scan_Counter);LCD_ShowString(80, 20, LCD_Buf);
                    LCD_Manager.Flag_LCD_Busy = false;
                    GNSS_Manager.GNSS_Scan_Counter_Prev = GNSS_Manager.GNSS_Scan_Counter;
                }

                // Waiting for RFID data
                if(RFID_Manager.Flag_RFID_Data_Waiting == true)
                {
                    if(RFID_Manager.Flag_RFID_RX_Begin == false)
                    {
                        UARTprintf("[RFID] Waiting for data, time: %d s\r\n", RFID_Manager.RFID_Data_Waiting_Counter + 1);

                        // Display to LCD
                        LCD_Manager.Flag_LCD_Busy = true;
                        //LCD_Fill(0,40,319,60, BLACK);
                        POINT_COLOR=YELLOW;
                        memset(LCD_Buf, 0, sizeof(LCD_Buf));sprintf(LCD_Buf, "[RFID] Waiting for data, time: %d s", RFID_Manager.RFID_Data_Waiting_Counter + 1);LCD_ShowString(0, 40, LCD_Buf);
                        LCD_Manager.Flag_LCD_Busy = false;
                    }

                    RFID_Manager.RFID_Data_Waiting_Counter++;
                    if(RFID_Manager.RFID_Data_Waiting_Counter >= 10)
                    {
                        RFID_Manager.Flag_RFID_Sending_Inprocess = false;
                        // Display to LCD
                        GPIOPinWrite(LCD_LED_BASE, LCD_LED_PIN, LCD_LED_PIN);
                        LCD_Manager.Flag_LCD_Busy = true;
                        LCD_Fill(0,40,319,60, BLACK);
                        memset(LCD_Buf, 0, sizeof(LCD_Buf));sprintf(LCD_Buf, "[RFID] Waiting for data timeout");LCD_ShowString(0, 40, LCD_Buf);
                        LCD_Manager.Flag_LCD_Busy = false;

                        RFID_Manager.RFID_Data_Waiting_Counter = 0;
                        RFID_Manager.Flag_RFID_Data_Waiting = false;

                        GNSS_Task_Execute();
                    }
                }
                else
                {
                    RFID_Manager.RFID_Data_Waiting_Counter = 0;
                }
            }
	    }
	}
}
//------------------------------------------------------------------------------------------------------------------------------------//
void ConfigureTimer_A4(uint16_t timer_tick_ms)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);
    TimerConfigure(TIMER4_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER4_BASE, TIMER_A, (SysCtlClockGet() / timer_tick_ms) -1);
    IntEnable(INT_TIMER4A);
    TimerIntEnable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();
    TimerEnable(TIMER4_BASE, TIMER_A);
}
//------------------------------------------------------------------------------------------------------------------------------------//
void Timer4IntHandler(void)
{
    TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    Main_Task_Counter++;
    if(Main_Task_Counter >= 100)
    {
        Main_Task_Counter = 0;
        Data_Transmit_Period_Counter++;

        // Execute RFID task in period
        if(Data_Transmit_Period_Counter >= 30 && D_Flag_t.Task_Selection == FL_TASK_RFID)
        {
            WDT_Counter = 0;

            GPIOPinWrite(LCD_LED_BASE, LCD_LED_PIN, LCD_LED_PIN);

            if(Mqtt_Manager.mesIsBeingSent == PUBLISH_BUSY || Mqtt_Manager.flag_mqtt_ack_wait == true || Mqtt_Manager.connectStatus == MQTT_STT_DISCONNECTTING)// Wait for publish and subscribe ack done
            {
                Data_Transmit_Period_Counter = 30;
            }
            else
            {
                Data_Transmit_Period_Counter = 0;
            }

            // Execute RFID task
            if(Mqtt_Manager.flag_mqtt_connection_ready == true && Data_Transmit_Period_Counter == 0)
            {
                RFID_Manager.Flag_RFID_Sending_Inprocess = true;
                GNSS_Manager.flag_GNSS_pub_done = false;
                GNSS_EnableReading(false);
                LCD_Manager.Flag_LCD_Busy = true;
                LCD_Fill(0,40,319,140, BLACK);
                LCD_Manager.Flag_LCD_Busy = false;
                RFID_Task_Execute(true);
            }
        }

        // Execute CAM task in demand
        if(CAM_Param.Flag_CAM_Require == true)
        {
            CAM_TakePhoto();
        }

        // MQTT wait for publishing RFID data
        if(RFID_Manager.Flag_RFID_SD_Store_Done == true)
        {
           c_RFID_Publish();
        }

        // MQTT wait for subscribing data
        MQTT_WaitForACK_Handler();

        // Get date time
        DS3231_Get_Time_Date();
    }
}
//------------------------------------------------------------------------------------------------------------------------------------//
void PWM0IntHandler(void)
{
    PWMGenIntClear(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_LOAD);
}
//------------------------------------------------------------------------------------------------------------------------------------//
void ConfigureUART(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 921600, 16000000);

    IntEnable(INT_UART0);
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
	IntMasterEnable();
}
//------------------------------------------------------------------------------------------------------------------------------------//
void UART0IntHandler(void)
{
	uint32_t ui32Status;
	ui32Status = UARTIntStatus(UART0_BASE, true);
	UARTIntClear(UART0_BASE, ui32Status);
	while(UARTCharsAvail(UART0_BASE))
	{
		uint16_t RX_char = UARTCharGetNonBlocking(UART0_BASE);
		UARTCharPut(UART0_BASE, RX_char); //CR
	}
}
//------------------------------------------------------------------------------------------------------------------------------------//
void ConfigureADC(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
								 ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 3);
	ADCIntClear(ADC0_BASE, 3);
}

///////////////////////////////////////////////////////////////////////////
uint16_t GetADCValue(void)
{
	uint32_t pui32ADC0Value[1];
	uint32_t ADC_value = 0;
	ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ADCIntClear(ADC0_BASE, 3);
	ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);
	ADC_value = pui32ADC0Value[0];
	return ADC_value;
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
void ConfigurePWM_Stellaris(void)
{
	uint32_t ulPeriod = 20;
	uint32_t dutyCycle = 10;

	// Turn off LEDs
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);

	// Configure PB6 as T0CCP0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB6_T0CCP0);
	GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_6);

	// Configure timer
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM);
	TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod -1);
	TimerMatchSet(TIMER0_BASE, TIMER_A, dutyCycle); // PWM
	TimerEnable(TIMER0_BASE, TIMER_A);
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
void PWM_Para_Setup(float freq, float pulsewidth)
{
	uint32_t Period_Number, PW_Number;
	float Period_Number_Tmp = 80000.0/freq;
	Period_Number = (uint32_t)(Period_Number_Tmp * 1000);
	PW_Number     = (uint32_t)(60*pulsewidth/100);
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0,  Period_Number);
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PW_Number);
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
void GPIO_Unlock_Pins(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	//Set PF0 as input pin
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
	//Unlocks the GPIO pin PF0
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = GPIO_PIN_4;
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    //Set PF0 as input pin
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
    //Unlocks the GPIO pin PF0
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = GPIO_PIN_0;
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
    //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
void EC_Meter_Activate(uint8_t command)
{
	if(command == 1) GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
	else GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
void IntGPIOf(void)
{

}
//-------------------------------------------------------------------------------------------------------------------------------------------//
void IntGPIOe(void)
{

}
//-------------------------------------------------------------------------------------------------------------------------------------------//
void ConfigureTivaPWM(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinConfigure(GPIO_PF1_M1PWM5);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, 20);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, 10);
	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
uint32_t CRC_Update(uint32_t crc, const void *data, int data_len)
{
    const unsigned char *d = (const unsigned char *)data;
    uint32_t tbl_idx;

    while (data_len--)
    {
        tbl_idx = (crc >> 12) ^ (*d >> 4);
        crc = crc_table[tbl_idx & 0x0f] ^ (crc << 4);
        tbl_idx = (crc >> 12) ^ (*d >> 0);
        crc = crc_table[tbl_idx & 0x0f] ^ (crc << 4);
        d++;
    }
    return crc & 0xffff;
}
//-------------------------------------------------------------------------------------------------------------------------------------------//
void WatchdogIntHandler(void)
{
    //
    // If we have been told to stop feeding the watchdog, return immediately
    // without clearing the interrupt.  This will cause the system to reset
    // next time the watchdog interrupt fires.
    //
    if(!g_bFeedWatchdog)
    {
        return;
    }

    WDT_Counter++;
    if(WDT_Counter >= 24)
    {
        WDT_Counter = 0;
        SysCtlReset();
    }

    //
    // Clear the watchdog interrupt.
    //
    ROM_WatchdogIntClear(WATCHDOG0_BASE);
}
void Configure_WatchDogTimer(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    //
    // Enable the watchdog interrupt.
    //
    ROM_IntEnable(INT_WATCHDOG);

    //
    // Set the period of the watchdog timer.
    //
    ROM_WatchdogReloadSet(WATCHDOG0_BASE, 5 * ROM_SysCtlClockGet());

    //
    // Enable reset generation from the watchdog timer.
    //
    ROM_WatchdogResetEnable(WATCHDOG0_BASE);

    //
    // Enable the watchdog timer.
    //
    ROM_WatchdogEnable(WATCHDOG0_BASE);
}
