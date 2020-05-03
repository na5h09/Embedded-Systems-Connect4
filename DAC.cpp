// put implementations for functions, explain how it works
// put your names here, date
#include <stdint.h>
#include "DAC.h"
#include "../inc/tm4c123gh6pm.h"

// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
	//Intialize port b as output
	SYSCTL_RCGCGPIO_R |= 0x00000002;     // 1) activate clock for Port B
	volatile int dummy = 5;
	dummy++;
	GPIO_PORTB_DIR_R |= 0x3F;          // 5) PB0-PB5 out
	GPIO_PORTB_DEN_R |= 0x3F;          // 7) enable digital I/O on PB4-0
}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none
void DAC_Out(uint32_t data){
	GPIO_PORTB_DATA_R = data ; //
	 
}