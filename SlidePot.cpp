// SlidePot.cpp
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "SlidePot.h"
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ 
// lab 8
	int volatile delay;
	SYSCTL_RCGCGPIO_R |= 0x08; 					//1) Activate clock for Port D
	while((SYSCTL_PRGPIO_R&0x08) == 0){};
		GPIO_PORTD_DIR_R &= ~0x04;					//2) make PD2 an input
		GPIO_PORTD_AFSEL_R |= 0x04;				//3)Engable alternate fun on PD2
		GPIO_PORTD_DEN_R &= ~0x04;					//4) Disable digital I/O on PD2
		GPIO_PORTD_AMSEL_R |= 0x04; 				//5 enable analog fun on PD2
		SYSCTL_RCGCADC_R |= 0x01;					//Activate ADC0
		delay = SYSCTL_RCGCADC_R;
		delay = 0;
		delay = SYSCTL_RCGCADC_R;
		delay = 1;
		delay = SYSCTL_RCGCADC_R;
		delay = 2;
		delay = SYSCTL_RCGCADC_R;
		delay = 3;
		delay = SYSCTL_RCGCADC_R;
		delay = 4;
		delay = SYSCTL_RCGCADC_R;
		ADC0_PC_R = 0x01; 								//7) CONFIGURE FOR 125K
		ADC0_SSPRI_R = 0x0123;						//8)Seq 3 is highest priority
		ADC0_ACTSS_R &= ~0x00008;					//9) Diable sample sequncer 3
		ADC0_EMUX_R &=~0XF000;						//10) seq3 is software trigger
		ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+5; //11 Ain5 PD2 
		ADC0_SSCTL3_R = 0x0006;						//12) No Temp Sense, Yes IE0 End0 - on sample completion set a flag
		ADC0_IM_R &= ~0x0008;							//13) disable SS3 interupts
		ADC0_ACTSS_R |= 0x0008;						//14 enable sample sequencer
}

//------------ADCIn------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  

// lab 8

	uint32_t data;
	ADC0_PSSI_R = 0x0008;
	while((ADC0_RIS_R & 0x08)==0){};
		data = ADC0_SSFIFO3_R&0xFFF;
		ADC0_ISC_R = 0x0008; //clear
		return data;
}

// constructor, invoked on creation of class
// m and b are linear calibration coeffients 
SlidePot::SlidePot(uint32_t m, uint32_t b){
// lab 8
	data = 0;
	slope = m;
	offset = b;
	distance = 0;
	flag = 1;
}

void SlidePot::Save(uint32_t n){
// lab 9
	data = n;
	distance = Convert(n);
	flag = 1;
}
uint32_t SlidePot::Convert(uint32_t n){
  // lab 8
  	return 6 + (n/585)*16 ; 

}

void SlidePot::Sync(void){
// lab 8
	// 1) wait for semaphore flag to be nonzero
// 2) set semaphore flag to 0
	while(flag == 0) {}
		
	flag = 0;
	return;
}

uint32_t SlidePot::ADCsample(void){ // return ADC sample value (0 to 4095)

  // return last calculated ADC sample
  return data; // replace this with solution
}

uint32_t SlidePot::Distance(void){  // return distance value (0 to 2000), 0.001cm
// lab 8
  // return last calculated distance in 0.001cm
  return distance; // replace this with solution
}

uint32_t SlidePot::currentColumn(void){ //added this function so we can know what column we are on
	uint32_t colnum = ((distance - 6)/ 16);
	if(colnum < 6) {
		return colnum;
	} else {
		return 6;
	}
}


