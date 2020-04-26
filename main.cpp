// main.cpp
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10 in C++

// Last Modified: 1/17/2020 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2017

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2017

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "SlidePot.h"
#include "Images.h"
#include "UART.h"
#include "Timer0.h"
#include "Timer1.h"


SlidePot my(1500,0);

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);

// Creating a struct for the Sprite.
typedef enum {dead,alive} status_t;
struct sprite{
  uint32_t x;      // x coordinate
  uint32_t y;      // y coordinate
  const unsigned short *image; // ptr->image
  status_t life;            // dead/alive
};          

typedef struct sprite sprite_t;

void SysTick_Init(unsigned long period){
  //*** students write this ******
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = (period - 1) & 0x00FFFFFF;
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = 0x00000007;
}

void PortF_Init(void){
  //*** students write this ******
	SYSCTL_RCGCGPIO_R |= 0x20;
	volatile int delay;
	delay++;
	GPIO_PORTF_DIR_R |= 0x0E;
	GPIO_PORTF_DEN_R |= 0x0E;
}

sprite_t bill={60,9,SmallEnemy20pointB,alive};
sprite_t me={52, 50,PlayerShip0, alive};

uint32_t time = 0;
volatile uint32_t flag;
void background(void){
  flag = 1; // semaphore
 // if(bill.life == alive){
   // bill.y++;
  //}
  if(bill.y>155){
    bill.life = dead;
  }
	me.x = my.Distance();
	//ST7735_FillRect(0,40,100,10, 0x0000);
	
}
void clock(void){
  time++;
}
int main(void){
	DisableInterrupts();
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
	SysTick_Init(8000000);	//sample at 10Hz or 0.1s
  ST7735_InitR(INITR_REDTAB); 
  ADC_Init();        // turn on ADC, PD2, set channel to 5
  PortF_Init();
  Random_Init(1);
  Output_Init();
  Timer0_Init(&background,1600000); // 50 Hz
  Timer1_Init(&clock,80000000); // 1 Hz
  EnableInterrupts();
		ST7735_DrawBitmap(10,160,Grid,100,100);
//  ST7735_DrawBitmap(10, 151, Bunker0, 18,5);
//	ST7735_DrawBitmap(40, 151, Bunker0, 18,5);
//	ST7735_DrawBitmap(70, 151, Bunker0, 18,5);
//	ST7735_DrawBitmap(100, 151, Bunker0, 18,5);
	
	
//  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
//  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
//  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
//  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);
//  ST7735_DrawBitmap(100, 9, SmallEnemy30pointB, 16,10);
  while(bill.life == alive){
    while(flag==0){};
    flag = 0;	
		
		ST7735_DrawBitmap(5,160,Grid,115,100);
   // ST7735_DrawBitmap(me.x,me.y,bill.image,16,10);
	  ST7735_DrawBitmap(me.x, me.y, me.image, 10,10); // player ship middle bottom
		

  }

  ST7735_FillScreen(0x0000);            // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString((char*)"GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_SetTextColor(ST7735_WHITE);
  ST7735_OutString((char*)"Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString((char*)"Earthling!");
  ST7735_SetCursor(2, 4);
  ST7735_SetTextColor(ST7735_WHITE);
  while(1){
    while(flag==0){};
    flag = 0;
    ST7735_SetCursor(2, 4);
    ST7735_OutUDec(time);
  }

}



void SysTick_Handler(void){ // every sample
    //*** students write this ******
// should call ADC_In() and Sensor.Save
  //GPIO_PORTF_DATA_R ^= 0x0E;		//toggle LED heartbeat
  my.Save(ADC_In());
	
}

