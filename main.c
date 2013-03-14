/* 
 * File:   main.c
 * Author: Tom
 *
 * Created on February 27, 2013, 9:25 PM
 */

/************************
 *                      *
 *    Header Files      *
 *                      *
 ************************/
#include <stdio.h>
#include <stdlib.h>
#include <p30F1010.h>
#include <xc.h>
#include <math.h>
#include <libpic30.h>
#include <libq.h>

/************************
 *                      *
 *     Definitions      *
 *                      *
 ************************/
#define KI 0x0008
#define VREF 0x2000
#define _ISR __attribute__((__interrupt__,__auto_psv__))

/**************************
 *                     	 	*
 * C Function Prototypes 	*
 *                       	*
 *************************/
void init_core(void);
void clear_buffers(void);
void init_adc(void);
void init_pwm(void);
void start_ramp_up(void);
void updatePWM(_Q16 x);
extern _Q16 readADC(void);
extern int round_Q16_2_int(_Q16 x);


/**************************
 *                      	*
 *   	ISR Prototypes     	*
 *                      	*
 *************************/
void _ISR _ADCInterrupt(void);
void _ISR _PWMSpEventMatchInterrupt(void);

/**************************
 *                      	*
 * 	Configuration Macros 	*
 *                      	*
 *************************/
_FOSCSEL(FRC_PLL)
_FWDT(FWDTEN_OFF)
_FPOR(PWRT_4)
/**************************
 *                      	*
 *   	Global Variables   	*
 *                      	*
 *************************/
//filter coefficients
#include "IIR_Coeffs.txt"
_Q16 ref = 8; //reference signal set low to allow for ramp up
_Q16 y;
_Q16 u;
_Q16 error[N];


/*
 * 
 */
int main(int argc, char** argv) {

    init_core();
    clear_buffers();
    //init_adc();
    //init_pwm();
    start_ramp_up();
    TRISB = 0x0;
    PORTB = 0x0;

    while(1){
	updatePWM(ref);
        PORTA++;

    }


    return (EXIT_SUCCESS);
}

void init_core(void){

    CORCONbits.IF = 0;
    CORCONbits.US = 0;
 
}

void clear_buffers(void){
    int i;

    for(i=0; i<N; i++){
        error[i] = 0;
    }

}


void init_adc(void){

    TRISBbits.TRISB0 = 1; //set AN0 as input
    ADPCFGbits.PCFG0 = 0; // set to analog port
    ADCONbits.FORM = 0;//ADC outputs integer number
    ADCONbits.EIE = 1;// enable interrupt on converstion 1
    ADCONbits.ADCS = 3;// clock ADC at 13.3MHz
    ADSTAT = 0; // clear ADSTAT
    ADCPC0bits.IRQEN0 = 1;
    ADCPC0bits.TRGSRC0 = 4; //start conversion on PWM generator #1
    
    ADCONbits.ADON = 1; //turns on ADC

}

void init_pwm(void){
    
    PTPERbits.PTPER = 0x4AF; // set PWM freq = 100kHz
    SEVTCMPbits.SEVTCMP = 0x257;//set special trigger for half way through PWM period
    //used for update of I compensator
    IOCON1bits.PENH = 1; //enable PWMH
    IOCON1bits.PENL = 1; //enable PWML
    IOCON1bits.POLH = 0; //PWMH active high
    IOCON1bits.POLL = 0; //PWML active low
    IOCON1bits.PMOD = 0; // Complementary Output pair
		IOCON1bits.OVRENH = 1; //set PWMH to override
		IOCON1bits.OVRDAT = 0; //set sync MOSFET to off
    TRIG1 = 0x0008;      // Sets trigger to almost the beginning of the PWM cycle
    PWMCON1bits.MDCS = 1;// Sets Duty cycle to as low as possible.
    PWMCON1bits.DTC = 0;
    PWMCON1bits.IUE = 0;
    PWMCON1bits.TRGIEN = 1;
		MDC = 0x0008;
    DTR1 = 120;

}

void start_ramp_up(void){

	PR1 = 0x06D6; //set interrupt time to 250us
	IEC0bits.T1IE = 1; //enable interrupt
	T1CONbits.TON = 1;
	
}

void updatePWM(_Q16 x){
	
	if(x<8){
		MDC = 8;
		return;
		
	}
	if(x>66519){
		MDC = 66519;
		return;
	}

	MDC = round_Q16_2_int(x);
		return;
	
	
	
}


void _ISR _ADCInterrupt(void){

	IFS0bits.ADIF = 0;  
	y = readADC();
    
}

void _ISR _PWMSpEventMatchInterrupt(void){

    
	_Q16 error_in;
        int i;
	IFS1bits.PSEMIF = 0;
	
		error_in =  ref - y;
	
		error[0] = error_in;
		for(i=N-1; i>0; i--)
		{
			error[0] -= a[i]*error[i];
			u += b[i]*error[i];
			error[i] = error[i-1];
		}
		u += b[0]*error[0];
	
}

void _ISR _T1Interrupt(void){
	
	IFS0bits.T1IF = 0; //clear interrupt flag
	if (ref == VREF)
        {  //if ref reaches ref point
            T1CONbits.TON = 0; //stop soft start
	}else{
            ref++; //else increment ref
	}
	
}







