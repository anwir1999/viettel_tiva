/*
 * FIR_Lib.h
 *
 *  Created on: Nov 30, 2016
 *      Author: Quang Dan
 */

#ifndef DIGITALFILTER_LIB_H_
#define DIGITALFILTER_LIB_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
double 	 FIR_Filter(uint16_t data, double coeff[], uint16_t L, double tmp_arr[], uint8_t* filter_counter);
double 	 IIR_SOS(double input, uint16_t j, double (*A)[3], double (*B)[3], double (*ftr_Internal)[3]);
double	 IIR_CASCADE(double input, uint16_t k, double (*A)[3], double (*B)[3], double (*ftr_Internal)[3] );
void     LP_Hamming_Coeff_Gen(double LP_Coeff[] ,double fc ,uint16_t order, double fs);
void     HP_Hamming_Coeff_Gen(double HP_Coeff[] ,double fc ,uint16_t order, double fs);
void     IIR_LP_Butterworth_Coeff_Gen(double (*A)[3], double (*G)[3], double fpass, double fstop, double Apass, double Astop, double fs);
void     IIR_HP_Butterworth_Coeff_Gen(double (*A)[3], double (*G)[3], double fpass, double fstop, double Apass, double Astop, double fs);
uint16_t IIR_HP_Butterworth_ReturnFilterOrder(double fpass, double fstop, double Apass, double Astop, double fs);
uint16_t IIR_LP_Butterworth_ReturnFilterOrder(double fpass, double fstop, double Apass, double Astop, double fs);
#endif /* DIGITALFILTER_LIB_H_ */
