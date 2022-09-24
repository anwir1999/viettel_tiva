// IFABEX TECHNOLOGIES
// Customized functions only for several TFT LCD symbols


#include <stdint.h>
#include <stdbool.h>
#include "IFT_LCD_PenColor.h"
#include "IFTSPI2_2LCD.h"
#include "ColorTFTSymbols.h"

/////////////////Signal Bar indication: in grades of 20%, small is 23 pixels from top, large is 27///
// Location is always width-54 pixels to width-30 pixel//

void	SignalBar(int dbm,uint16_t barcolouron,uint16_t barcolouroff,uint16_t barcolourerr, int bartype)
{
		int	signal=0;
		signal = dbm;
		if(bartype == 1)
		{
			if(signal==0)
			{
				LCD_Fill((LCD_W-30),7,(LCD_W-29),27,barcolourerr);
				LCD_Fill((LCD_W-35),9,(LCD_W-34),27,barcolourerr);
				LCD_Fill((LCD_W-60),19,(LCD_W-59),27,barcolourerr);
				LCD_Fill((LCD_W-40),11,(LCD_W-39),27,barcolourerr);
				LCD_Fill((LCD_W-45),13,(LCD_W-44),27,barcolourerr);
				LCD_Fill((LCD_W-50),15,(LCD_W-49),27,barcolourerr);
				LCD_Fill((LCD_W-55),17,(LCD_W-54),27,barcolourerr);
			}
			else if((0>signal)&(signal<15))
			{
				LCD_Fill((LCD_W-30),7,(LCD_W-29),27,barcolouroff);
				LCD_Fill((LCD_W-35),9,(LCD_W-34),27,barcolouroff);
				LCD_Fill((LCD_W-60),19,(LCD_W-59),27,barcolouron);
				LCD_Fill((LCD_W-40),11,(LCD_W-39),27,barcolouroff);
				LCD_Fill((LCD_W-45),13,(LCD_W-44),27,barcolouroff);
				LCD_Fill((LCD_W-50),15,(LCD_W-49),27,barcolouroff);
				LCD_Fill((LCD_W-55),17,(LCD_W-54),27,barcolouroff);
			}
			else if((14>signal)&(signal<30))
				{
					LCD_Fill((LCD_W-30),7,(LCD_W-29),27,barcolouroff);
					LCD_Fill((LCD_W-35),9,(LCD_W-34),27,barcolouroff);
					LCD_Fill((LCD_W-40),11,(LCD_W-39),27,barcolouroff);
					LCD_Fill((LCD_W-45),13,(LCD_W-44),27,barcolouroff);
					LCD_Fill((LCD_W-50),15,(LCD_W-49),27,barcolouroff);
					LCD_Fill((LCD_W-55),17,(LCD_W-54),27,barcolouron);
					LCD_Fill((LCD_W-60),19,(LCD_W-59),27,barcolouron);
				}
			else if((29>signal)&(signal<65))
				{
					LCD_Fill((LCD_W-30),7,(LCD_W-29),27,barcolouroff);
					LCD_Fill((LCD_W-35),9,(LCD_W-34),27,barcolouroff);
					LCD_Fill((LCD_W-40),11,(LCD_W-39),27,barcolouroff);
					LCD_Fill((LCD_W-45),13,(LCD_W-44),27,barcolouroff);
					LCD_Fill((LCD_W-50),15,(LCD_W-49),27,barcolouron);
					LCD_Fill((LCD_W-55),17,(LCD_W-54),27,barcolouron);
					LCD_Fill((LCD_W-60),19,(LCD_W-59),27,barcolouron);
				}
			else if((64>signal)&(signal<70))
				{
					LCD_Fill((LCD_W-30),7,(LCD_W-29),27,barcolouroff);
					LCD_Fill((LCD_W-35),9,(LCD_W-34),27,barcolouroff);
					LCD_Fill((LCD_W-40),11,(LCD_W-39),27,barcolouroff);
					LCD_Fill((LCD_W-45),13,(LCD_W-44),27,barcolouron);
					LCD_Fill((LCD_W-50),15,(LCD_W-49),27,barcolouron);
					LCD_Fill((LCD_W-55),17,(LCD_W-54),27,barcolouron);
					LCD_Fill((LCD_W-60),19,(LCD_W-59),27,barcolouron);
				}
			else if((69<signal)&(signal<80))
				{
					LCD_Fill((LCD_W-30),7,(LCD_W-29),27,barcolouroff);
					LCD_Fill((LCD_W-35),9,(LCD_W-34),27,barcolouroff);
					LCD_Fill((LCD_W-40),11,(LCD_W-39),27,barcolouron);
					LCD_Fill((LCD_W-45),13,(LCD_W-44),27,barcolouron);
					LCD_Fill((LCD_W-50),15,(LCD_W-49),27,barcolouron);
					LCD_Fill((LCD_W-55),17,(LCD_W-54),27,barcolouron);
					LCD_Fill((LCD_W-60),19,(LCD_W-59),27,barcolouron);
				}
			else if((79<signal)&(signal<90))
				{
					LCD_Fill((LCD_W-30),7,(LCD_W-29),27,barcolouroff);
					LCD_Fill((LCD_W-35),9,(LCD_W-34),27,barcolouron);
					LCD_Fill((LCD_W-40),11,(LCD_W-39),27,barcolouron);
					LCD_Fill((LCD_W-45),13,(LCD_W-44),27,barcolouron);
					LCD_Fill((LCD_W-50),15,(LCD_W-49),27,barcolouron);
					LCD_Fill((LCD_W-55),17,(LCD_W-54),27,barcolouron);
					LCD_Fill((LCD_W-60),19,(LCD_W-59),27,barcolouron);
				}

			else if((91<signal)&(signal<100))
				{
					LCD_Fill((LCD_W-30),7,(LCD_W-29),27,barcolouron);
					LCD_Fill((LCD_W-35),9,(LCD_W-34),27,barcolouron);
					LCD_Fill((LCD_W-40),11,(LCD_W-39),27,barcolouron);
					LCD_Fill((LCD_W-45),13,(LCD_W-44),27,barcolouron);
					LCD_Fill((LCD_W-50),15,(LCD_W-49),27,barcolouron);
					LCD_Fill((LCD_W-55),17,(LCD_W-54),27,barcolouron);
					LCD_Fill((LCD_W-60),19,(LCD_W-59),27,barcolouron);
				}
		}

		if(bartype == 2)
		{
			if(signal==0)
			{
				LCD_Fill((LCD_W-60),19,(LCD_W-59),23,barcolourerr);
				LCD_Fill((LCD_W-40),11,(LCD_W-39),23,barcolourerr);
				LCD_Fill((LCD_W-45),13,(LCD_W-44),23,barcolourerr);
				LCD_Fill((LCD_W-50),15,(LCD_W-49),23,barcolourerr);
				LCD_Fill((LCD_W-55),17,(LCD_W-54),23,barcolourerr);
			}
			else if((0>signal)&(signal<20))
			{
				LCD_Fill((LCD_W-60),19,(LCD_W-59),23,barcolouron);
				LCD_Fill((LCD_W-40),11,(LCD_W-39),23,barcolouroff);
				LCD_Fill((LCD_W-45),13,(LCD_W-44),23,barcolouroff);
				LCD_Fill((LCD_W-50),15,(LCD_W-49),23,barcolouroff);
				LCD_Fill((LCD_W-55),17,(LCD_W-54),23,barcolouroff);
			}
			else if((19>signal)&(signal<40))
				{
					LCD_Fill((LCD_W-40),11,(LCD_W-39),23,barcolouroff);
					LCD_Fill((LCD_W-45),13,(LCD_W-44),23,barcolouroff);
					LCD_Fill((LCD_W-50),15,(LCD_W-49),23,barcolouroff);
					LCD_Fill((LCD_W-55),17,(LCD_W-54),23,barcolouron);
					LCD_Fill((LCD_W-60),19,(LCD_W-59),23,barcolouron);
				}
			else if((39>signal)&(signal<60))
				{
					LCD_Fill((LCD_W-40),11,(LCD_W-39),23,barcolouroff);
					LCD_Fill((LCD_W-45),13,(LCD_W-44),23,barcolouroff);
					LCD_Fill((LCD_W-50),15,(LCD_W-49),23,barcolouron);
					LCD_Fill((LCD_W-55),17,(LCD_W-54),23,barcolouron);
					LCD_Fill((LCD_W-60),19,(LCD_W-59),23,barcolouron);
				}
			else if((59>signal)&(signal<80))
				{
					LCD_Fill((LCD_W-40),11,(LCD_W-39),23,barcolouroff);
					LCD_Fill((LCD_W-45),13,(LCD_W-44),23,barcolouron);
					LCD_Fill((LCD_W-50),15,(LCD_W-49),23,barcolouron);
					LCD_Fill((LCD_W-55),17,(LCD_W-54),23,barcolouron);
					LCD_Fill((LCD_W-60),19,(LCD_W-59),23,barcolouron);
				}
			else if((79<signal)&(signal<100))
				{
					LCD_Fill((LCD_W-40),11,(LCD_W-39),23,barcolouron);
					LCD_Fill((LCD_W-45),13,(LCD_W-44),23,barcolouron);
					LCD_Fill((LCD_W-50),15,(LCD_W-49),23,barcolouron);
					LCD_Fill((LCD_W-55),17,(LCD_W-54),23,barcolouron);
					LCD_Fill((LCD_W-60),19,(LCD_W-59),23,barcolouron);
				}
		}

}

//////////////Battery BAR, Charging will show power flash sign in black, Discharging will show full Green, Empty will show RED
void		Battery(charge,status) // charge = % of charge measured from ADC; status = charger connected=1, discharging = 2
{
	if(status==2)
		{
			LCD_Fill((LCD_W-120),15,(LCD_W-90),27,GREEN);
			LCD_Fill((LCD_W-90),17,(LCD_W-86),25,GREEN);
			LCD_Fill((LCD_W-118),17,(LCD_W-92),25,BLACK);
		}
}
