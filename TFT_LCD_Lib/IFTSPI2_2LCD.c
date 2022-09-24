#include <stdint.h>
#include <stdbool.h>



#include "IFTSPI2_2LCD.h"
#include "IFT_LCD_PenColor.h"
#include "IFT_LCD_2_2_font.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "inc/hw_gpio.h"
#include "driverlib/pin_map.h"

#define MADCTL_MY  0x80  ///< Bottom to top
#define MADCTL_MX  0x40  ///< Right to left
#define MADCTL_MV  0x20  ///< Reverse Mode
#define MADCTL_ML  0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00  ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08  ///< Blue-Green-Red pixel order
#define MADCTL_MH  0x04  ///< LCD refresh right to left

uint16_t _width  = ILI9341_TFTWIDTH;
uint16_t _height = ILI9341_TFTHEIGHT;

char LCD_Buf[200];
LCD_Manager_t LCD_Manager;


uint32_t ClockFunction(void)
{

	uint32_t ui32SysClock;

	ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                SYSCTL_CFG_VCO_480), 120000000);

	return(ui32SysClock);
}

void	notrequired(void)
{

}

void LCD_Writ_Bus(char da)   //Parallel Data Write function
{
		char bitdata;
		bitdata=da;
		SSIDataPut(SSI2_BASE,bitdata);
		while(SSIBusy(SSI2_BASE));

}
void LCD_WR_DATA8_SSI(char da) //Send -8 bit parameter data  // CONVERTED TO CCS
{
    //DC=1;
    GPIOPinWrite(GPIO_PORTB_BASE, LCD_DC_PIN, LCD_DC_PIN); //Pulses the dc line
	LCD_Writ_Bus(da);
}
 void LCD_WR_DATA(int da) // CONVERTED TO CCS
{
	//DC=1;
	GPIOPinWrite(GPIO_PORTB_BASE, LCD_DC_PIN, LCD_DC_PIN); //Pulses the dc line
	LCD_Writ_Bus(da>>8);
	LCD_Writ_Bus(da);
}
void LCD_WR_REG(char da)	 // CONVERTED TO CCS
{
    //DC=0;
	GPIOPinWrite(GPIO_PORTB_BASE, LCD_DC_PIN, 0); //Pulses the dc line
	LCD_Writ_Bus(da);
}
void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)  // CONVERTED TO CCS
{
	   LCD_WR_REG(0x2a);
	   LCD_WR_DATA8_SSI(x1>>8);
	   LCD_WR_DATA8_SSI(x1);
	   LCD_WR_DATA8_SSI(x2>>8);
	   LCD_WR_DATA8_SSI(x2);

	   LCD_WR_REG(0x2b);
	   LCD_WR_DATA8_SSI(y1>>8);
	   LCD_WR_DATA8_SSI(y1);
	   LCD_WR_DATA8_SSI(y2>>8);
	   LCD_WR_DATA8_SSI(y2);

	   LCD_WR_REG(0x2C);
}
void Lcd_Init(void)
{
    //LCD_Manager.Flag_LCD_Busy = false;
	//GPIOPinWrite(GPIO_PORTA_BASE, CS,CS);
	GPIOPinWrite(GPIO_PORTB_BASE, LCD_CS_PIN, LCD_CS_PIN);
    //TFT_RST=1;
	//GPIOPinWrite(GPIO_PORTE_BASE, RESET,0x20);
	GPIOPinWrite(GPIO_PORTB_BASE, LCD_RST_PIN, LCD_RST_PIN);
    //delay_ms(5);
	SysCtlDelay(SysCtlClockGet()/100);// 5 ms delay
    //TFT_RST=0;
	//GPIOPinWrite(GPIO_PORTE_BASE, RESET,0);
	GPIOPinWrite(GPIO_PORTB_BASE, LCD_RST_PIN, 0);
    //delay_ms(15);
	SysCtlDelay(SysCtlClockGet()/100);// 15 ms delay
    //TFT_RST=1;
	//GPIOPinWrite(GPIO_PORTE_BASE, RESET,0x20);
	GPIOPinWrite(GPIO_PORTB_BASE, LCD_RST_PIN, LCD_RST_PIN);
    //delay_ms(15);
	SysCtlDelay(SysCtlClockGet()/100);// 15 ms delay
    //TFT_CS =0;
	//GPIOPinWrite(GPIO_PORTA_BASE, CS,0x08);
	GPIOPinWrite(GPIO_PORTB_BASE, LCD_CS_PIN, LCD_CS_PIN);
	SysCtlDelay(SysCtlClockGet()/100);// 15 ms delay
	GPIOPinWrite(GPIO_PORTB_BASE, LCD_CS_PIN, 0);
	//GPIOPinWrite(GPIO_PORTA_BASE, CS,0);

	LCD_WR_REG(0xCB);
    LCD_WR_DATA8_SSI(0x39);
    LCD_WR_DATA8_SSI(0x2C);
    LCD_WR_DATA8_SSI(0x00);
    LCD_WR_DATA8_SSI(0x34);
    LCD_WR_DATA8_SSI(0x02);

    LCD_WR_REG(0xCF);
    LCD_WR_DATA8_SSI(0x00);
    LCD_WR_DATA8_SSI(0XC1);
    LCD_WR_DATA8_SSI(0X30);

    LCD_WR_REG(0xE8);
    LCD_WR_DATA8_SSI(0x85);
    LCD_WR_DATA8_SSI(0x00);
    LCD_WR_DATA8_SSI(0x78);

    LCD_WR_REG(0xEA);
    LCD_WR_DATA8_SSI(0x00);
    LCD_WR_DATA8_SSI(0x00);

    LCD_WR_REG(0xED);
    LCD_WR_DATA8_SSI(0x64);
    LCD_WR_DATA8_SSI(0x03);
    LCD_WR_DATA8_SSI(0X12);
    LCD_WR_DATA8_SSI(0X81);

    LCD_WR_REG(0xF7);
    LCD_WR_DATA8_SSI(0x20);

    LCD_WR_REG(0xC0);    //Power control
    LCD_WR_DATA8_SSI(0x23);   //VRH[5:0]

    LCD_WR_REG(0xC1);    //Power control
    LCD_WR_DATA8_SSI(0x10);   //SAP[2:0];BT[3:0]

    LCD_WR_REG(0xC5);    //VCM control
    LCD_WR_DATA8_SSI(0x3e);
    LCD_WR_DATA8_SSI(0x28);

    LCD_WR_REG(0xC7);    //VCM control2
    LCD_WR_DATA8_SSI(0x86);  //--

    LCD_WR_REG(0x36);    // Memory Access Control
    LCD_WR_DATA8_SSI(0x48);

    LCD_WR_REG(0x3A);
    LCD_WR_DATA8_SSI(0x55);

    LCD_WR_REG(0xB1);
    LCD_WR_DATA8_SSI(0x00);
    LCD_WR_DATA8_SSI(0x18);

    LCD_WR_REG(0xB6);    // Display Function Control
    LCD_WR_DATA8_SSI(0x08);
    LCD_WR_DATA8_SSI(0x82);
    LCD_WR_DATA8_SSI(0x27);

    LCD_WR_REG(0xF2);    // 3Gamma Function Disable
    LCD_WR_DATA8_SSI(0x00);

    LCD_WR_REG(0x26);    //Gamma curve selected
    LCD_WR_DATA8_SSI(0x01);

    LCD_WR_REG(0xE0);    //Set Gamma
    LCD_WR_DATA8_SSI(0x0F);
    LCD_WR_DATA8_SSI(0x31);
    LCD_WR_DATA8_SSI(0x2B);
    LCD_WR_DATA8_SSI(0x0C);
    LCD_WR_DATA8_SSI(0x0E);
    LCD_WR_DATA8_SSI(0x08);
    LCD_WR_DATA8_SSI(0x4E);
    LCD_WR_DATA8_SSI(0xF1);
    LCD_WR_DATA8_SSI(0x37);
    LCD_WR_DATA8_SSI(0x07);
    LCD_WR_DATA8_SSI(0x10);
    LCD_WR_DATA8_SSI(0x03);
    LCD_WR_DATA8_SSI(0x0E);
    LCD_WR_DATA8_SSI(0x09);
    LCD_WR_DATA8_SSI(0x00);

    LCD_WR_REG(0XE1);    //Set Gamma
    LCD_WR_DATA8_SSI(0x00);
    LCD_WR_DATA8_SSI(0x0E);
    LCD_WR_DATA8_SSI(0x14);
    LCD_WR_DATA8_SSI(0x03);
    LCD_WR_DATA8_SSI(0x11);
    LCD_WR_DATA8_SSI(0x07);
    LCD_WR_DATA8_SSI(0x31);
    LCD_WR_DATA8_SSI(0xC1);
    LCD_WR_DATA8_SSI(0x48);
    LCD_WR_DATA8_SSI(0x08);
    LCD_WR_DATA8_SSI(0x0F);
    LCD_WR_DATA8_SSI(0x0C);
    LCD_WR_DATA8_SSI(0x31);
    LCD_WR_DATA8_SSI(0x36);
    LCD_WR_DATA8_SSI(0x0F);

    LCD_WR_REG(0x11);    //Exit Sleep
 //   delayms(120);
	SysCtlDelay(SysCtlClockGet()/20);// 200 ms delay

    LCD_WR_REG(0x29);    //Display on
    LCD_WR_REG(0x2c);

    SysCtlDelay(SysCtlClockGet()/100);

    LCD_Clear(BLACK);
	BACK_COLOR=BLACK;
	POINT_COLOR=RED;

	setRotation(1);

}

void LCD_Clear(u16 Color) // CONVERTED TO CCS
{
    //LCD_Manager.Flag_LCD_Busy = true;
	u8 VH,VL;
	u16 i,j;
	VH=Color>>8;
	VL=Color;

	Address_set(0,0,_width,_height);

    for(i=0;i<_width;i++)
    {
	  for (j=0;j<_height;j++) // LCD_H
	   	{

        	 LCD_WR_DATA8_SSI(VH);
			 LCD_WR_DATA8_SSI(VL);
	    }
    }
    //LCD_Manager.Flag_LCD_Busy = false;

}
////Dotted
////POINT_COLOR:The color of this point
void LCD_DrawPoint(u16 x,u16 y)
{
	Address_set(x,y,x,y);//Setting the cursor position
	LCD_WR_DATA(POINT_COLOR);
}

// Fill in the designated area specified color
// Size of the area:
//  (xend-xsta)*(yend-ysta)
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color) // CONVERTED TO CCS
{
    //LCD_Manager.Flag_LCD_Busy = true;
	u16 i,j;
	Address_set(xsta,ysta,xend,yend);      //Setting the cursor position
	for(i=ysta;i<=yend;i++)
	{
		for(j=xsta;j<=xend;j++)LCD_WR_DATA(color);//Setting the cursor position
	}
	//LCD_Manager.Flag_LCD_Busy = false;
}
// Draw the line
//x1,y1:Starting point coordinates
//x2,y2:End coordinates
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2) // CONVERTED TO CCS
{
    //LCD_Manager.Flag_LCD_Busy = true;
	u16 t;
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;

	delta_x=x2-x1; //Calculate the coordinates of the incremental
	delta_y=y2-y1;
	uRow=x1;
	uCol=y1;
	if(delta_x>0)incx=1; //Set single-step directions
	else if(delta_x==0)incx=0;//Vertical line
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if(delta_y==0)incy=0;//Level
	else{incy=-1;delta_y=-delta_y;}
	if( delta_x>delta_y)distance=delta_x; //Select the basic incremental axis
	else distance=delta_y;
	for(t=0;t<=distance+1;t++ )//Drawing a line output
	{
		LCD_DrawPoint(uRow,uCol);//Dotted
		xerr+=delta_x ;
		yerr+=delta_y ;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
	//LCD_Manager.Flag_LCD_Busy = false;
}
//Draw a rectangle
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2) // CONVERTED TO CCS
{
    //LCD_Manager.Flag_LCD_Busy = true;
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
	//LCD_Manager.Flag_LCD_Busy = false;
}
//A circle the size of the appointed position draw
//(x,y):The center
//r    :Radius
void Draw_Circle(u16 x0,u16 y0,u8 r)  // CONVERTED TO CCS
{
    //LCD_Manager.Flag_LCD_Busy = true;
	int a,b;
	int di;
	a=0;b=r;
	di=3-(r<<1);             //Judgment flag next point position
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a);             //3
		LCD_DrawPoint(x0+b,y0-a);             //0
		LCD_DrawPoint(x0-a,y0+b);             //1
		LCD_DrawPoint(x0-b,y0-a);             //7
		LCD_DrawPoint(x0-a,y0-b);             //2
		LCD_DrawPoint(x0+b,y0+a);             //4
		LCD_DrawPoint(x0+a,y0-b);             //5
		LCD_DrawPoint(x0+a,y0+b);             //6
		LCD_DrawPoint(x0-b,y0+a);
		a++;
		//Using the Bresenham algorithm Circle
		if(di<0)di +=4*a+6;
		else
		{
			di+=10+4*(a-b);
			b--;
		}
		LCD_DrawPoint(x0+a,y0+b);
	}
	//LCD_Manager.Flag_LCD_Busy = false;
}
////Displays a character at the specified position
//
//// num "" ---> "~"
//// mode: overlay mode (1) or non-overlapping mode (0)
//// Display a character at the specified location
//
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 mode) // CONVERTED TO CCS
{
    u8 temp;
    u8 pos,t;
	u16 x0=x;
	u16 colortemp=POINT_COLOR;
    if(x>_width-16||y>_height-16)
    	{
    		return;
    	}
	//Settings window
	num=num-' ';//Obtained after the offset value
	Address_set(x,y,x+8-1,y+16-1);      //Setting the cursor position
	if(!mode) //Non-overlapping mode
	{
		for(pos=0;pos<16;pos++)
		{
			temp=asc2_1608[(u16)num*16+pos];		 //Call 1608 fonts
			for(t=0;t<8;t++)
		    {
		        if(temp&0x01)POINT_COLOR=colortemp;
				else POINT_COLOR=BACK_COLOR;
				LCD_WR_DATA(POINT_COLOR);
				temp>>=1;
				x++;
		    }
			x=x0;
			y++;
		}
	}else//Superimposition
	{
		for(pos=0;pos<16;pos++)
		{
		    temp=asc2_1608[(u16)num*16+pos];		 //Call 1608 fonts
			for(t=0;t<8;t++)
		    {
		        if(temp&0x01)LCD_DrawPoint(x+t,y+pos);//Draw a point
		        temp>>=1;
		    }
		}
	}
	POINT_COLOR=colortemp;
}
// m ^ n function
u32 mypow(u8 m,u8 n)
{
	u32 result=1;
	while(n--)result*=m;
	return result;
}
// Show two figures
// x, y: starting point coordinates
// len: Digits
// color: color
// num: value (0 to 4294967295);
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len) // CONVERTED TO CCS
{
	u8 t,temp;
	u8 enshow=0;
	num=(u16)num;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+8*t,y,' ',0);
				continue;
			}else enshow=1;

		}
	 	LCD_ShowChar(x+8*t,y,temp+48,0);
	}
}
// Show two figures
// x, y: starting point coordinates
// num: number (0 to 99);
void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len) // CONVERTED TO CCS
{
	u8 t,temp;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
	 	LCD_ShowChar(x+8*t,y,temp+'0',0);
	}
}
// Display the string
// x, y: starting point coordinates
// * p: string starting address
// With 16 fonts
void LCD_ShowString(u16 x,u16 y, char *p) // CONVERTED TO CCS
{
    //LCD_Manager.Flag_LCD_Busy = true;
    while(*p!='\0')
    {
        if(x>_width-16){x=0;y+=16;}
        if(y>_height-16)
        {
        	y=x=0;
        	LCD_Clear(RED);
        }
        LCD_ShowChar(x,y,*p,0);
        x+=8;
        p++;
    }
    //LCD_Manager.Flag_LCD_Busy = false;
}
void setRotation(uint8_t m)
{
	uint8_t rotation = m % 4; // can't be higher than 3
    switch (rotation) {
        case 0:
            m = (MADCTL_MX | MADCTL_BGR);
            //_width  = ILI9341_TFTWIDTH;
            //_height = ILI9341_TFTHEIGHT;
            break;
        case 1:
            m = (MADCTL_MV | MADCTL_BGR);
            _width  = 320;
            _height = 240;
            break;
        case 2:
            m = (MADCTL_MY | MADCTL_BGR);
            //_width  = ILI9341_TFTWIDTH;
            //_height = ILI9341_TFTHEIGHT;
            break;
        case 3:
            m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
            //_width  = ILI9341_TFTHEIGHT;
			//_height = ILI9341_TFTWIDTH;
            break;
    }
    LCD_WR_REG(ILI9341_MADCTL);
    LCD_WR_DATA8_SSI(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////ENABLING PORTS//////////////////////////////////////////////
void TivaInit(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);// SSI pins

	GPIOPinTypeGPIOOutput(LCD_RST_BASE, LCD_RST_PIN);
	GPIOPinTypeGPIOOutput(LCD_CS_BASE, LCD_CS_PIN);
	GPIOPinTypeGPIOOutput(LCD_DC_BASE, LCD_DC_PIN);
	GPIOPinTypeGPIOOutput(LCD_LED_BASE, LCD_LED_PIN);

	GPIOPinWrite(LCD_LED_BASE, LCD_LED_PIN, LCD_LED_PIN);

/////////////////SSI CONFIG HERE//////////////////////////////////////////////////////
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB4_SSI2CLK);
	//GPIOPinConfigure(GPIO_PB5_SSI2FSS);
	GPIOPinConfigure(GPIO_PB6_SSI2RX);
	GPIOPinConfigure(GPIO_PB7_SSI2TX);

	// The pins are assigned as follows:
	//      PB7 - SSI2Tx
	//      PB6 - SSI2Rx
	//      PB5 - SSI2Fss
	//      PB4 - SSI2CLK
	// TODO: change this to select the port/pin you are using.
	GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_7 | GPIO_PIN_6 |  GPIO_PIN_4);
	//GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);
	SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 40000000, 8);
	SSIEnable(SSI2_BASE);
////////////////////// SSI CONFIG ENDS/////////////////////////////////////////
}
/////
