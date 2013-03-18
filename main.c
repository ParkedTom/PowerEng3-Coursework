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
#define VREF 0x02000000
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
_FOSC(PRIOSC_OFF & FRC_HI_RANGE)
/**************************
 *                      	*
 *   	Global Variables   	*
 *                      	*
 *************************/
//filter coefficients
#include "IIR_Coeffs.txt"
_Q16 ref = 0x00080000; //reference signal set low to allow for ramp up
_Q16 y;
_Q16 u;
_Q16 error[N];


/*
 * 
 */
void main(void) {

    init_core();
    clear_buffers();
    init_adc();
    init_pwm();
    start_ramp_up();
   // TRISB = 0x0;
   // PORTB = 0x0;

		PDC1 = 0x1290;
    while(1){
		//updatePWM(ref);
 	   // PORTB++;
		//updatePWM(250);

    }

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
	IEC0bits.ADIE = 1;
    ADCONbits.ADCS = 3;// clock ADC at 13.3MHz
    ADSTAT = 0; // clear ADSTAT
    ADCPC0bits.IRQEN0 = 1;
    ADCPC0bits.TRGSRC0 = 4; //start conversion on PWM generator #1
    
    ADCONbits.ADON = 1; //turns on ADC

}

void init_pwm(void){
   	PTCON = 0x0800;
	PTPER = 0x2534;
	PHASE1 = 0x0000;
	PWMCON1 = 0x0001;
	FCLCON1 = 0x0003;
	IOCON1 = 0xC000;
	TRISEbits.TRISE1 = 0;
	PORTEbits.RE1 = 0;
	PDC1 = 0x04A6;
	DTR1 = 0x0040;
	ALTDTR1 = 0x0040;
	TRIG1 = 0x00FF;
	PTCONbits.PTEN = 1;

}

void start_ramp_up(void){

	PR1 = 0x06D6; //set interrupt time to 250us
	IEC0bits.T1IE = 1; //enable interrupt
	T1CONbits.TON = 1;
	
}

void updatePWM(_Q16 x){
	
	int int_x;

	int_x = (signed) round_Q16_2_int(x); 	

	if(int_x<8){
		PDC1 = 8;
		return;
		
	}
	if(int_x>9504){
		PDC1 = 9504;
		return;
	}

	PDC1 = int_x;
		return;
	
	
	
}


void _ISR _ADCInterrupt(void){

	_Q16 error_in;
	int i;

	IFS0bits.ADIF = 0;  
	ADSTAT = 0;
	y = readADC();
	error_in =  ref - y;
	
	error[0] = error_in;
	for(i=N-1; i>0; i--)
	{
		//_Q16mac(-a[i], error[i], error[0]);
		error[0] -= error[i]*a[i];
		//_Q16mac(b[i],error[i],u);
		u += error[i]*b[i];
		error[i] = error[i-1];
	}
	//_Q16mac(b[0], error[0], u);
	u += error[0]*b[0];
    
}



void _ISR _T1Interrupt(void){
	
	IFS0bits.T1IF = 0; //clear interrupt flag
	if (ref == VREF)
        {  //if ref reaches ref point
            T1CONbits.TON = 0; //stop soft start
	}else{
           ref = ref + 0x00010000; //else increment ref
	}
	
}







