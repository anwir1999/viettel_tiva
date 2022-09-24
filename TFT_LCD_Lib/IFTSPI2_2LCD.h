/*
 * clockfunc.h
 *
 *  Created on: Nov 18, 2015
 *      Author: a0876236
 */



#ifndef IFTSPI2_2LCD_H_
#define IFTSPI2_2LCD_H_



#define		AllPins							GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
#define		TFTControlPins					GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
#define		TFTReset						GPIO_PIN_4
#define		LPLEDs							GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
#define		u8 								unsigned char
#define		u16 							unsigned int
#define		u32 							unsigned long
#define		MSB								GPIO_PORTB_BASE
#define		LSB								GPIO_PORTD_BASE
// SSI RELATED DEFINITIONS, ONLY FOR 2.2" LCD //
#define		SCK								GPIO_PIN_2//PORT A, SSI 0 ONLY
#define		CS								GPIO_PIN_3
#define		MISO							GPIO_PIN_4
#define		MOSI							GPIO_PIN_5
#define		DC								GPIO_PIN_4// PORT E
#define		RESET							GPIO_PIN_5// PORT E


#define LCD_CS_PERIPH           SYSCTL_PERIPH_GPIOB
#define LCD_CS_BASE             GPIO_PORTB_BASE
#define LCD_CS_PIN              GPIO_PIN_5

#define LCD_DC_PERIPH           SYSCTL_PERIPH_GPIOB
#define LCD_DC_BASE             GPIO_PORTB_BASE
#define LCD_DC_PIN              GPIO_PIN_1

#define LCD_RST_PERIPH           SYSCTL_PERIPH_GPIOB
#define LCD_RST_BASE             GPIO_PORTB_BASE
#define LCD_RST_PIN              GPIO_PIN_0

#define LCD_LED_PERIPH           SYSCTL_PERIPH_GPIOB
#define LCD_LED_BASE             GPIO_PORTB_BASE
#define LCD_LED_PIN              GPIO_PIN_2
//---------------------------------------------------------------------------------------------------------------//
#define ILI9341_TFTWIDTH   240      ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT  320      ///< ILI9341 max TFT height

#define ILI9341_NOP        0x00     ///< No-op register
#define ILI9341_SWRESET    0x01     ///< Software reset register
#define ILI9341_RDDID      0x04     ///< Read display identification information
#define ILI9341_RDDST      0x09     ///< Read Display Status

#define ILI9341_SLPIN      0x10     ///< Enter Sleep Mode
#define ILI9341_SLPOUT     0x11     ///< Sleep Out
#define ILI9341_PTLON      0x12     ///< Partial Mode ON
#define ILI9341_NORON      0x13     ///< Normal Display Mode ON

#define ILI9341_RDMODE     0x0A     ///< Read Display Power Mode
#define ILI9341_RDMADCTL   0x0B     ///< Read Display MADCTL
#define ILI9341_RDPIXFMT   0x0C     ///< Read Display Pixel Format
#define ILI9341_RDIMGFMT   0x0D     ///< Read Display Image Format
#define ILI9341_RDSELFDIAG 0x0F     ///< Read Display Self-Diagnostic Result

#define ILI9341_INVOFF     0x20     ///< Display Inversion OFF
#define ILI9341_INVON      0x21     ///< Display Inversion ON
#define ILI9341_GAMMASET   0x26     ///< Gamma Set
#define ILI9341_DISPOFF    0x28     ///< Display OFF
#define ILI9341_DISPON     0x29     ///< Display ON

#define ILI9341_CASET      0x2A     ///< Column Address Set
#define ILI9341_PASET      0x2B     ///< Page Address Set
#define ILI9341_RAMWR      0x2C     ///< Memory Write
#define ILI9341_RAMRD      0x2E     ///< Memory Read

#define ILI9341_PTLAR      0x30     ///< Partial Area
#define ILI9341_VSCRDEF    0x33     ///< Vertical Scrolling Definition
#define ILI9341_MADCTL     0x36     ///< Memory Access Control
#define ILI9341_VSCRSADD   0x37     ///< Vertical Scrolling Start Address
#define ILI9341_PIXFMT     0x3A     ///< COLMOD: Pixel Format Set

#define ILI9341_FRMCTR1    0xB1     ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2    0xB2     ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3    0xB3     ///< Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR     0xB4     ///< Display Inversion Control
#define ILI9341_DFUNCTR    0xB6     ///< Display Function Control

#define ILI9341_PWCTR1     0xC0     ///< Power Control 1
#define ILI9341_PWCTR2     0xC1     ///< Power Control 2
#define ILI9341_PWCTR3     0xC2     ///< Power Control 3
#define ILI9341_PWCTR4     0xC3     ///< Power Control 4
#define ILI9341_PWCTR5     0xC4     ///< Power Control 5
#define ILI9341_VMCTR1     0xC5     ///< VCOM Control 1
#define ILI9341_VMCTR2     0xC7     ///< VCOM Control 2

#define ILI9341_RDID1      0xDA     ///< Read ID 1
#define ILI9341_RDID2      0xDB     ///< Read ID 2
#define ILI9341_RDID3      0xDC     ///< Read ID 3
#define ILI9341_RDID4      0xDD     ///< Read ID 4

#define ILI9341_GMCTRP1    0xE0     ///< Positive Gamma Correction
#define ILI9341_GMCTRN1    0xE1     ///< Negative Gamma Correction
//#define ILI9341_PWCTR6     0xFC

// Color definitions
#define ILI9341_BLACK       0x0000  ///<   0,   0,   0
#define ILI9341_NAVY        0x000F  ///<   0,   0, 123
#define ILI9341_DARKGREEN   0x03E0  ///<   0, 125,   0
#define ILI9341_DARKCYAN    0x03EF  ///<   0, 125, 123
#define ILI9341_MAROON      0x7800  ///< 123,   0,   0
#define ILI9341_PURPLE      0x780F  ///< 123,   0, 123
#define ILI9341_OLIVE       0x7BE0  ///< 123, 125,   0
#define ILI9341_LIGHTGREY   0xC618  ///< 198, 195, 198
#define ILI9341_DARKGREY    0x7BEF  ///< 123, 125, 123
#define ILI9341_BLUE        0x001F  ///<   0,   0, 255
#define ILI9341_GREEN       0x07E0  ///<   0, 255,   0
#define ILI9341_CYAN        0x07FF  ///<   0, 255, 255
#define ILI9341_RED         0xF800  ///< 255,   0,   0
#define ILI9341_MAGENTA     0xF81F  ///< 255,   0, 255
#define ILI9341_YELLOW      0xFFE0  ///< 255, 255,   0
#define ILI9341_WHITE       0xFFFF  ///< 255, 255, 255
#define ILI9341_ORANGE      0xFD20  ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define ILI9341_PINK        0xFC18  ///< 255, 130, 198

typedef struct
{
    bool Flag_LCD_Busy;
}LCD_Manager_t;

// Variables //
uint32_t	SysFreq;
int32_t	LCD_READ;// read_data=0;
unsigned char bitdata;
uint32_t reply[];
static u16 kkk=1 , kkkbk=0;


extern  u16 BACK_COLOR, POINT_COLOR;


extern uint32_t ClockFunction(void);
extern	void	notrequired(void);
//extern	void	useless(void);
extern void TivaInit(void);
extern void Lcd_Init(void);
extern void LCD_Clear(u16 Color);
extern void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2);
extern void LCD_WR_DATA8(char da);
extern void LCD_WR_DATA(int da);
extern void LCD_WR_REG(char da);

extern void LCD_DrawPoint(u16 x,u16 y);
extern void LCD_DrawPoint_big(u16 x,u16 y);
u16  LCD_ReadPoint(u16 x,u16 y);
extern void Draw_Circle(u16 x0,u16 y0,u8 r);
extern void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
extern void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);
extern void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color);
extern void LCD_ShowChar(u16 x,u16 y,u8 num,u8 mode);
extern void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len);
extern void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len);
extern void LCD_ShowString(u16 x,u16 y,char *p);
extern	void	notrequired(void);
void setRotation(uint8_t m);



#endif /* CLOCKFUNC_H_ */
