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
#define KI 0x0001
#define VREF 0x02000000
#define _ISR __attribute__((__interrupt__,__auto_psv__))
#define PWM_FULLSCALE 0x2534
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
void updatePWM(signed int x);
extern _Q15 readADC(void);
extern int round_Q16_2_int(_Q16 x);
extern signed int I_controller_update(signed int, signed int);


/**************************
 *                      	*
 *   	ISR Prototypes     	*
 *                      	*
 *************************/
void _ISR _ADCInterrupt(void);
void _ISR _PWMSpEventMatchInterrupt(void);
void _ISR _INT1Interrupt(void);

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
signed int ref = 0x8000; //reference signal set low to allow for ramp up
signed int y;
signed int u[2];
int overflow;
//_Q16 error[N];


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


    while(1){
		//updatePWM(ref);
 	   // PORTB++;
		updatePWM(u[0]);
	//	ref++;

		if(!PORTAbits.RA9){
			IOCON1bits.PENH = 0;
			IOCON1bits.PENL = 0;
		}
    }

}

void init_core(void){

    CORCONbits.IF = 0;
    CORCONbits.US = 0;
	CORCONbits.SATA = 1;
	CORCONbits.SATB = 1;
	CORCONbits.ACCSAT = 1;
	CORCONbits.SATDW = 1;
	CORCONbits.RND = 0;
	INTCON2bits.INT1EP = 1;
	IEC1bits.INT1IE = 1;
	TRISDbits.TRISD0 = 1;
 
}

void clear_buffers(void){
    int i;

    u[0] = 0;
	u[1] = 0;

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
//	TRISEbits.TRISE1 = 0;
//	PORTEbits.RE1 = 0;
	PDC1 = 0x04A6;
	DTR1 = 0x0100;
	ALTDTR1 = 0x0100;
	TRIG1 = 0x00FF;
	PTCONbits.PTEN = 1;

}

void start_ramp_up(void){

	PR1 = 0x06D6; //set interrupt time to 250us
	IEC0bits.T1IE = 1; //enable interrupt
	T1CONbits.TON = 1;
	
}

void updatePWM(signed int x){
	

	if(x<950){
		PDC1 =  PWM_FULLSCALE - 950;
		return;
		
	}
	if(x>7619){
		PDC1 = PWM_FULLSCALE - 7619;
		return;
	}

	PDC1 = PWM_FULLSCALE - x;
		return;
	
	
	
}


void _ISR _ADCInterrupt(void){

	signed int error_in;
	int i;

	IFS0bits.ADIF = 0;  
	ADSTAT = 0;
	y = readADC();
	error_in =  ref - y;
	
	u[1] = u[0];
//	u[0] = u[1] + KI*error_in;
	u[0] = I_controller_update(u[1], error_in); 
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

void _ISR _INT1Interrupt(void){
	
	IOCON1bits.PENH = 0;
	IOCON1bits.PENL = 0;
	TRISEbits.TRISE0 = 0;
	TRISEbits.TRISE1 = 0;
	PORTEbits.RE0 = 0;
	PORTEbits.RE1 = 1;
	IFS1bits.INT1IF = 0;
}






