/*
 * IFT_LCD_PenColor.h
 *	Pen Color Headers to be used only for the IFABEX PRODUCTS LCD
 *  Created on: Nov 18, 2015
 *      Author: a0876236
 */



#ifndef IFT_LCD_PenColor_H_
#define IFT_LCD_PenColor_H_

#define LCD_W 240
#define LCD_H 320

//Pen color	16 BIT COLOR PALLETTE	// Color Bits = RRRR RGGG GGGB BBBB
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000
#define BLUE         	 0x001F // 0000 0000 0001 1111
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800 // 1111 1000 0000 0000
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0 // 0000 0111 1110 0000
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //Brown
#define BRRED 			 0XFC07 //Brownish red
#define GRAY  			 0X8430 //Gray
//GUI Color

#define DARKBLUE      	 0X01CF	//Navy blue
#define LIGHTBLUE      	 0X7D7C	//Light Blue
#define GRAYBLUE       	 0X5458 //Gray-blue
// More than three colors for color PANEL

#define LIGHTGREEN     	 0X841F //Light green
#define LGRAY 			 0XC618 //Light gray (PANNEL), form the background color

#define LGRAYBLUE        0XA651 //Light gray blue (intermediate layer color)
#define LBBLUE           0X2B12 //Blue light brown (select entry inverse)


#endif /* IFT_LCD_PenColor_H_ */
