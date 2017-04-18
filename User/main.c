/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "stm32f10x.h"                  		// Device header
#include "rgb_control.h"

// Luminous fluxes  
float red_lm = 1.315; 		// maximum brightness of red, measured in lab.
float green_lm	= 2.553;	// maximum brightness of green, measured in lab.
float blue_lm = 0.466;  // maximum brightness of blue, measured in lab.
float target_xy[2] = {0.13, 0.07}; //SET TARGET COLOR HERE
float brightness = 1; //brightness between 0 and 1

void RGB_ratio(float xy[], float dutyCycles[]){
	float xR = 0.6907;	//Red x
	float yR = 0.3092;	//Red y
	float xG = 0.1516;	//Green x
	float yG = 0.7445;	//Green y
	float xB = 0.1296;	//Blue x
	float yB = 0.0627; 	//Blue y
	
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
	
	float dutyCycleRed = dutyCycles[0] * brightness;
	float dutyCycleGreen = dutyCycles[1] * brightness;
	float dutyCycleBlue = dutyCycles[2] * brightness;
	osThreadCreate(osThread(Green), &dutyCycleGreen); 	
	osThreadCreate(osThread(Red), &dutyCycleRed); 	
	osThreadCreate(osThread(Blue), &dutyCycleBlue); 	
  osKernelStart ();                         // start thread execution 
}


