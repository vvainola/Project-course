#include "rgb_control.h"
#include "delay.h"

void Red (void const *dutyCycle) {
	unsigned onDelayRed = *(float*)dutyCycle * 1000;
	unsigned offDelayRed = 1000-onDelayRed;
	while (1) {
		GPIOB->BSRR = (1<<13);			//LED on: set PortB.13
		delayUS_DWT(onDelayRed);					//pause
		GPIOB->BRR = (1<<13);				//LED off: clear PortB.13
		delayUS_DWT(offDelayRed);					//pause
	}
}

void Blue (void const *dutyCycle) {
	unsigned onDelayBlue = *(float*)dutyCycle * 1000;
	unsigned offDelayBlue = 1000-onDelayBlue;
	while (1) {
		GPIOB->BSRR = (1<<11);			//LED on: set PortB.11
		delayUS_DWT(onDelayBlue);	
		GPIOB->BRR = (1<<11);				//LED off: clear PortB.11
		delayUS_DWT(offDelayBlue);				
	}
}

void Green (void const *dutyCycle) {
	unsigned onDelayGreen = *(float*)dutyCycle * 1000;
	unsigned offDelayGreen = 1000-onDelayGreen;
	while (1) {
		GPIOB->BSRR = (1<<15);			//LED on: set PortB.15
		delayUS_DWT(onDelayGreen);
		GPIOB->BRR = (1<<15);				//LED off: clear PortB.15
		delayUS_DWT(offDelayGreen);
	}
}

