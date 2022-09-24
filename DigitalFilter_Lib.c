/*
 * FIR_Lib.c
 *
 *  Created on: Nov 30, 2016
 *      Author: Quang Dan
 */

#include "DigitalFilter_Lib.h"
#define PI 					3.14159265358979323846
///////////////////////////////////////////////////////////////////////////
double FIR_Filter(uint16_t data, double coeff[], uint16_t L, double tmp_arr[], uint8_t* filter_counter)
{
	double result = 0;
	uint16_t i;
	(*filter_counter)++;
	for(i = 0; i < L; i++)
	{
		if(i < (L-1))
		{
			tmp_arr[i] = data * coeff[i] + tmp_arr[i+1];
		}
		else
		{
			tmp_arr[i] = data * coeff[i];
		}
	}
	if(*filter_counter == L)
	{
		result = tmp_arr[0];
		(*filter_counter)--;
	}
	return result;
}
///////////////////////////////////////////////////////////////////////////
double IIR_SOS(double input, uint16_t j, double (*A)[3], double (*B)[3], double (*ftr_Internal)[3])
{
	double result = 0;
	ftr_Internal[j][0] = input - A[j][1] * ftr_Internal[j][1] - A[j][2] * ftr_Internal[j][2];
	result = B[j][0] * ftr_Internal[j][0] + B[j][1] * ftr_Internal[j][1] + B[j][2] * ftr_Internal[j][2];
	ftr_Internal[j][2] = ftr_Internal[j][1];
	ftr_Internal[j][1] = ftr_Internal[j][0];
	return result;
}
///////////////////////////////////////////////////////////////////////////
double IIR_CASCADE(double input, uint16_t k, double (*A)[3], double (*B)[3], double (*ftr_Internal)[3] )
{
	double result = 0;
	double tmp_result = 0;
	uint16_t i;
	tmp_result = input;
	//*(*(A + 0) + 0) = 100;
	for(i = 0; i <k; i++)
	{
		tmp_result = IIR_SOS(tmp_result , i, A, B, ftr_Internal);
	}
	result = tmp_result;
	return result;
}
///////////////////////////////////////////////////////////////////////////
void LP_Hamming_Coeff_Gen(double LP_Coeff[] ,double fc ,uint16_t order, double fs)
{
	uint16_t i;
	uint16_t M = order/2;
	uint16_t N= order +1;
	double wc = (2*3.14*fc)/fs;
	double HW = 0;
	double Norm_sum = 0;
	for(i = 0; i < N; i++)
	{
		HW = 0.54 - 0.46*cos(2*3.14*i/order);
		if(i != M)
		{
			LP_Coeff[i] = (HW*sin(wc*(i-M))/(3.14*(i-M)));
		}
		else
		{
			LP_Coeff[i] = HW*wc/3.14;
		}
		Norm_sum += LP_Coeff[i];
	}
	for(i = 0; i < N; i++)
	{
		LP_Coeff[i] = LP_Coeff[i] / Norm_sum;
	}
}
///////////////////////////////////////////////////////////////////////////
void HP_Hamming_Coeff_Gen(double HP_Coeff[] ,double fc ,uint16_t order, double fs)
{
	uint16_t i;
	uint16_t M = order/2;
	uint16_t N= order +1;
	double wc = (2*3.14*fc)/fs;
	double HW = 0;
	double Norm_sum = 0;
	for(i = 0; i < N; i++)
	{
		HW = 0.54 - 0.46*cos(2*3.14*i/order);
		if(i != M)
		{
			HP_Coeff[i] = (-HW*sin(wc*(i-M))/(3.14*(i-M)));
		}
		else
		{
			HP_Coeff[i] = HW*(1-wc/3.14);
		}
		Norm_sum += HP_Coeff[i];
	}
	for(i = 0; i < N; i++)
	{
		HP_Coeff[i] = HP_Coeff[i] / Norm_sum;
	}
}
///////////////////////////////////////////////////////////////////////////
uint16_t IIR_HP_Butterworth_ReturnFilterOrder(double fpass, double fstop, double Apass, double Astop, double fs)
{
	uint16_t IIR_FilterOrder = 0;
	double wpass = 2*PI*fpass/fs;
	double wstop = 2*PI*fstop/fs;

	double opass = 1/tan(wpass/2);
	double ostop = 1/tan(wstop/2);

	double epass = sqrt(pow(10,Apass/10)-1);
	double estop = sqrt(pow(10,Astop/10)-1);

	double Nexact = log(estop/epass)/log(ostop/opass);
	IIR_FilterOrder = (uint16_t)(Nexact + 1);
	return IIR_FilterOrder;
}
///////////////////////////////////////////////////////////////////////////
void IIR_HP_Butterworth_Coeff_Gen(double (*A)[3], double (*G)[3], double fpass, double fstop, double Apass, double Astop, double fs)

{
	double wpass = 2*PI*fpass/fs;
	double wstop = 2*PI*fstop/fs;

	double opass = 1/tan(wpass/2);
	double ostop = 1/tan(wstop/2);

	double epass = sqrt(pow(10,Apass/10)-1);
	double estop = sqrt(pow(10,Astop/10)-1);

	double Nexact = log(estop/epass)/log(ostop/opass);
	uint16_t Napproximate = (uint16_t)(Nexact + 1);
	double Napproximate_double = (double)(Napproximate);

	double norm_freq = opass/ pow(epass, (1/(double)(Napproximate)));
	uint16_t i,j;
	double cos_phi = 0;
	if(Napproximate%2 == 0) // Báº­c cháºµn
	{
		for(i = 0; i < (Napproximate/2); i++)
		{
			cos_phi = cos(PI*(Napproximate_double-1 + (double)(2*(i+1)))/ (2*Napproximate_double));
			for(j = 0; j< 3; j++)
			{
				if(j ==1)
				{
					G[i][j] = -2*(norm_freq*norm_freq)/(1-2*norm_freq*cos_phi + norm_freq*norm_freq);

					A[i][j] = -2*(norm_freq*norm_freq -1)/(1 - 2*norm_freq*cos_phi + norm_freq*norm_freq);
				}
				else
				{
					G[i][j] = (norm_freq*norm_freq)/(1-2*norm_freq*cos_phi + norm_freq*norm_freq);
					if(j == 0)
					{
						A[i][j] = 1;
					}
					else // j==2
					{
						A[i][j] = (1 + 2*norm_freq*cos_phi + norm_freq*norm_freq)/ (1 - 2*norm_freq*cos_phi + norm_freq*norm_freq);
					}
				}
			}
		}
	}
	else // Báº­c láº»
	{
		for(i = 0; i < (Napproximate/2 + 1); i++)
		{
			if(i == 0) // CÃ¡c pháº§n tá»­ Ä‘áº§u tiÃªn cá»§a cÃ¡c máº£ng há»‡ sá»‘ bá»™ lá»�c IIR
			{
				for(j = 0; j< 3; j++)
				{
					if(j ==1)// j=1
					{
						G[i][j] = -norm_freq/(norm_freq +1);

						A[i][j] = -(norm_freq -1)/(norm_freq+1);
					}
					else
					{
						if(j == 0)// j=0
						{
							A[i][j] = 1;
							G[i][j] = norm_freq/(norm_freq +1);
						}
						else // j=2
						{
							A[i][j] = 0;
							G[i][j] = 0;
						}
					}
				}
			}
			else // CÃ¡c pháº§n tá»­ tiáº¿p theo cá»§a cÃ¡c máº£ng há»‡ sá»‘ bá»™ lá»�c IIR
			{
				cos_phi = cos(PI*(Napproximate_double-1 + (double)(2*i))/ (2*Napproximate_double));
				for(j = 0; j< 3; j++)
				{
					if(j ==1) // j =1
					{
						G[i][j] = -2*(norm_freq*norm_freq)/(1-2*norm_freq*cos_phi + norm_freq*norm_freq);

						A[i][j] = -2*(norm_freq*norm_freq -1)/(1 - 2*norm_freq*cos_phi + norm_freq*norm_freq);
					}
					else
					{
						G[i][j] = (norm_freq*norm_freq)/(1-2*norm_freq*cos_phi + norm_freq*norm_freq);
						if(j == 0) // j= 0;
						{
							A[i][j] = 1;
						}
						else // j=2
						{
							A[i][j] = (1 + 2*norm_freq*cos_phi + norm_freq*norm_freq)/ (1 - 2*norm_freq*cos_phi + norm_freq*norm_freq);
						}
					}
				}
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////
uint16_t IIR_LP_Butterworth_ReturnFilterOrder(double fpass, double fstop, double Apass, double Astop, double fs)
{
	uint16_t IIR_FilterOrder = 0;
	double wpass = 2*PI*fpass/fs;
	double wstop = 2*PI*fstop/fs;

	double opass = tan(wpass/2);
	double ostop = tan(wstop/2);

	double epass = sqrt(pow(10,Apass/10)-1);
	double estop = sqrt(pow(10,Astop/10)-1);

	double Nexact = log(estop/epass)/log(ostop/opass);
	IIR_FilterOrder = (uint16_t)(Nexact + 1);
	return IIR_FilterOrder;
}
///////////////////////////////////////////////////////////////////////////
void IIR_LP_Butterworth_Coeff_Gen(double (*A)[3], double (*G)[3], double fpass, double fstop, double Apass, double Astop, double fs)

{
	double wpass = 2*PI*fpass/fs;
	double wstop = 2*PI*fstop/fs;

	double opass = tan(wpass/2);
	double ostop = tan(wstop/2);

	double epass = sqrt(pow(10,Apass/10)-1);
	double estop = sqrt(pow(10,Astop/10)-1);

	double Nexact = log(estop/epass)/log(ostop/opass);
	uint16_t Napproximate = (uint16_t)(Nexact + 1);
	double Napproximate_double = (double)(Napproximate);

	double norm_freq = opass/ pow(epass, (1/(double)(Napproximate)));
	uint16_t i,j;
	double cos_phi = 0;
	if(Napproximate%2 == 0) // Báº­c cháºµn
	{
		for(i = 0; i < (Napproximate/2); i++)
		{
			cos_phi = cos(PI*(Napproximate_double-1 + (double)(2*(i+1)))/ (2*Napproximate_double));
			for(j = 0; j< 3; j++)
			{
				if(j ==1)
				{
					G[i][j] = 2*(norm_freq*norm_freq)/(1-2*norm_freq*cos_phi + norm_freq*norm_freq);

					A[i][j] = 2*(norm_freq*norm_freq -1)/(1 - 2*norm_freq*cos_phi + norm_freq*norm_freq);
				}
				else
				{
					G[i][j] = (norm_freq*norm_freq)/(1-2*norm_freq*cos_phi + norm_freq*norm_freq);
					if(j == 0)
					{
						A[i][j] = 1;
					}
					else // j==2
					{
						A[i][j] = (1 + 2*norm_freq*cos_phi + norm_freq*norm_freq)/ (1 - 2*norm_freq*cos_phi + norm_freq*norm_freq);
					}
				}
			}
		}
	}
	else // Báº­c láº»
	{
		for(i = 0; i < (Napproximate/2 + 1); i++)
		{
			if(i == 0) // CÃ¡c pháº§n tá»­ Ä‘áº§u tiÃªn cá»§a cÃ¡c máº£ng há»‡ sá»‘ bá»™ lá»�c IIR
			{
				for(j = 0; j< 3; j++)
				{
					if(j ==1)// j=1
					{
						G[i][j] = norm_freq/(norm_freq +1);

						A[i][j] = (norm_freq -1)/(norm_freq+1);
					}
					else
					{
						if(j == 0)// j=0
						{
							A[i][j] = 1;
							G[i][j] = norm_freq/(norm_freq +1);
						}
						else // j=2
						{
							A[i][j] = 0;
							G[i][j] = 0;
						}
					}
				}
			}
			else // CÃ¡c pháº§n tá»­ tiáº¿p theo cá»§a cÃ¡c máº£ng há»‡ sá»‘ bá»™ lá»�c IIR
			{
				cos_phi = cos(PI*(Napproximate_double-1 + (double)(2*i))/ (2*Napproximate_double));
				for(j = 0; j< 3; j++)
				{
					if(j ==1) // j =1
					{
						G[i][j] = 2*(norm_freq*norm_freq)/(1-2*norm_freq*cos_phi + norm_freq*norm_freq);

						A[i][j] = 2*(norm_freq*norm_freq -1)/(1 - 2*norm_freq*cos_phi + norm_freq*norm_freq);
					}
					else
					{
						G[i][j] = (norm_freq*norm_freq)/(1-2*norm_freq*cos_phi + norm_freq*norm_freq);
						if(j == 0) // j= 0;
						{
							A[i][j] = 1;
						}
						else // j=2
						{
							A[i][j] = (1 + 2*norm_freq*cos_phi + norm_freq*norm_freq)/ (1 - 2*norm_freq*cos_phi + norm_freq*norm_freq);
						}
					}
				}
			}
		}
	}
}

