/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "stm32f10x.h"                  		// Device header
#include <stdlib.h>

// Luminous fluxes  
float red_lm = 1.15; 		// maximum brightness of red, measured in lab.
float green_lm	= 2.3;	// maximum brightness of green, measured in lab.
float blue_lm = 0.396;  // maximum brightenss of blue, measured in lab.
float target_xy[2] = {0.3, 0.5}; //SET TARGET COLOR HERE
float brightness = 0.5; //brightness between 0 and 1
float dutyCycleRed;
float dutyCycleGreen;
float dutyCycleBlue;


void RGB_ratio(float xy[], float dutyCycles[]){
	float xR = 0.693;	//Red x
	float yR = 0.307;	//Red y
	float xG = 0.147;	//Green x
	float yG = 0.755;	//Green y
	float xB = 0.1285;//Blue x
	float yB = 0.062; //Blue y
	
	// For derivation of ratios see 
	// http://www.ledsmagazine.com/articles/print/volume-10/issue-6/features/understand-rgb-led-mixing-ratios-to-realize-optimal-color-in-signs-and-displays-magazine.html
	float xD = xy[0];
	float yD = xy[1];
	float mRB = (yR - yB)/(xR-xB);
	float cRB = yB - mRB*xB;

	float mGD = (yG-yD)/(xG-xD);
	float cGD = yG-mGD*xG;

	float xP = (cGD-cRB)/(mRB-mGD);
	float yP = mRB * xP + cRB;

	float R_RB = -(yR/yB)*(yB-yP) / (yR-yP);
	float R_GP = -(yG/yP)*(yP-yD) / (yG-yD);
	float ratio_R = (R_RB)/(R_RB+1); 	//Red
	float ratio_G = R_GP;							//Green
	float ratio_B = 1/(R_RB+1);				//Blue
	float total_ratio = ratio_R + ratio_G + ratio_B;
	dutyCycles[0] = (blue_lm / total_ratio * ratio_R)/red_lm; 	//Red duty cycle
	dutyCycles[1] = (blue_lm / total_ratio * ratio_G)/green_lm; //Green duty cycle
	dutyCycles[2] = (blue_lm / total_ratio * ratio_B)/blue_lm;	//Blue duty cycle
}

void delayUS_DWT(uint32_t us) {
	volatile uint32_t cycles = (SystemCoreClock/1000000L)*us;
	volatile uint32_t start = DWT->CYCCNT;
	do  {
	} while(DWT->CYCCNT - start < cycles);
}

void Red (void const *argument) {
	unsigned onDelayRed = dutyCycleRed*1000;
	unsigned offDelayRed = 1000-onDelayRed;
	while (1) {
		GPIOB->BSRR = (1<<13);			//LED on: set PortB.13
		delayUS_DWT(onDelayRed);					//pause
		GPIOB->BRR = (1<<13);				//LED off: clear PortB.13
		delayUS_DWT(offDelayRed);					//pause
	}
}

void Blue (void const *argument) {
	unsigned onDelayBlue = dutyCycleBlue*1000;
	unsigned offDelayBlue = 1000-onDelayBlue;
	while (1) {
		GPIOB->BSRR = (1<<14);			//LED on: set PortB.14
		delayUS_DWT(onDelayBlue);	
		GPIOB->BRR = (1<<14);				//LED off: clear PortB.14
		delayUS_DWT(offDelayBlue);				
	}
}

void Green (void const *argument) {
	unsigned onDelayGreen = dutyCycleGreen*1000;
	unsigned offDelayGreen = 1000-onDelayGreen;
	while (1) {
		GPIOB->BSRR = (1<<15);			//LED on: set PortB.15
		delayUS_DWT(onDelayGreen);
		GPIOB->BRR = (1<<15);				//LED off: clear PortB.15
		delayUS_DWT(offDelayGreen);
	}
}

osThreadDef(Red, osPriorityNormal, 1, 0); //Define Blinky Thread
osThreadDef(Blue, osPriorityNormal, 1, 0); //Define Blinky Thread
osThreadDef(Green, osPriorityNormal, 1, 0); //Define Blinky Thread

/*
 * main: initialize and start the system
 */
int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS

  // initialize peripherals here
	RCC->APB2ENR |= (1UL << 3);                /* Enable GPIOB clock            */
	GPIOB->CRH    =  0x33333333;               /* PB.8..16 defined as Outputs   */
	
	volatile uint32_t *DWT_CONTROL = (uint32_t *) 0xE0001000;
	volatile uint32_t *DWT_CYCCNT = (uint32_t *) 0xE0001004;
	volatile uint32_t *DEMCR = (uint32_t *) 0xE000EDFC;
	volatile uint32_t *LAR  = (uint32_t *) 0xE0001FB0;   // <-- added lock access register

	*DEMCR = *DEMCR | 0x01000000;     // enable trace
	*LAR = 0xC5ACCE55;                // <-- added unlock access to DWT (ITM, etc.)registers 
	*DWT_CYCCNT = 0;                  // clear DWT cycle counter
	*DWT_CONTROL = *DWT_CONTROL | 1;  // enable DWT cycle counter
  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);
	
	float dutyCycles[3];
	
	RGB_ratio(target_xy, dutyCycles);
	
	dutyCycleRed = dutyCycles[0] * brightness;
	dutyCycleGreen = dutyCycles[1] * brightness;
	dutyCycleBlue = dutyCycles[2] * brightness;
	osThreadCreate(osThread(Green), NULL); 	
	osThreadCreate(osThread(Red), NULL); 	
	osThreadCreate(osThread(Blue), NULL); 	
  osKernelStart ();                         // start thread execution 
}

/* 
Luminous fluxes: 
Red 	32 	lm
Green	51	lm
Blue 	8 	lm

Wavelengths:
Red		617 nm
Green 525 nm
Blue	467 nm
*/



