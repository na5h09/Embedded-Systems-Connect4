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
#include "Sound.h"


SlidePot my(1500,0);
bool spanishFlag = false;

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);

// Creating a struct for the Sprite.
struct Node{
	bool filled;
	uint32_t x;
	uint32_t y;
	uint16_t color;
	const unsigned short *coin; //0 is none, 0xCA47 is blue, 0x20FD is red
};
typedef struct Node Gridspace;
Gridspace Grid[7][6];
uint16_t blue_c = 0xCA47;
uint16_t red_c = 0x20FD;

class token{
	private:
		uint32_t x;      // x coordinate
		uint32_t prevX;
		uint32_t y;      // y coordinate
		uint32_t col;
		const unsigned short *image; // ptr->image
	
	public:
		token(){
				x = 0;
		}
		
		void Init(uint32_t x1, uint32_t y1, const unsigned short *p) {
				x = x1; y = y1; image = p;
				prevX = x; 
		}
		
		void changeDistance(uint32_t value) {
			x = value;
		}
		
		void currentCol(uint32_t value) {
			col = value;
		}
		uint32_t getCol() {
			return col;
		}
		
		void drawMe(void){
			if(prevX != x){
				ST7735_FillRect(prevX,y - 10,10,11,0x0000);
			}
			ST7735_DrawBitmap(x, y, image, 10,10);
			prevX = x;
	
		}
		
			
};          

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

void PortE_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x10; //Port E clock on
	volatile int dumbass;
	GPIO_PORTE_DIR_R &= 0xFC; //PE0 & PE1 IN
	GPIO_PORTE_DEN_R |= 0x03;//
}

bool tokenSelect;
token blue;
bool playerWin, compWin, drawFlag;
uint32_t grid_count;
uint32_t Player_Score = 0;
uint32_t Comp_Score = 0;
void GameInit(){
	blue.Init(100, 55, blueChip);
	for(int i = 0; i < 7; i++) {
		for(int j = 0; j < 6; j++) {
			Grid[i][j].filled = false;
			Grid[i][j].color = 0;
			Grid[i][j].coin = 0;
			Grid[i][j].x = (16*i) + 9;
			Grid[i][j].y = (16*j) + 75;
		}
	}
	playerWin = false;
	drawFlag = false;
	compWin = false;
	grid_count = 0;
}

void GameDraw() {
	ST7735_SetCursor(0, 0);
	if(spanishFlag ){
	ST7735_OutString("PUNTACION JUGADOR: ");
	ST7735_OutUDec(Player_Score);
	ST7735_SetCursor(0, 1);
	ST7735_OutString("PUNTUACION COMPUTADORA: ");
	ST7735_OutUDec(Comp_Score);	
	}
	else{
	ST7735_OutString("Player Score: ");
	ST7735_OutUDec(Player_Score);
	ST7735_SetCursor(0, 1);
	ST7735_OutString("Computer Score: ");
	ST7735_OutUDec(Comp_Score);
	}
	blue.drawMe();
	for(int i = 0; i < 7; i++) {
		for(int j = 0; j < 6; j++) {
			if(Grid[i][j].filled == true) {
				ST7735_DrawBitmap(Grid[i][j].x, Grid[i][j].y, Grid[i][j].coin, 10, 10);
			}
		}
	}
}	


void delay() {
	for(int i = 7999999; i > 0; i--) {}
}

volatile uint32_t flag; //different flag conditions
void placeToken(uint32_t column, uint16_t tok_color, const unsigned short *c_coin) { //places token in column
	for(int i = 5; i >=0; i--) {
			if(Grid[column][i].filled == false){
				Grid[column][i].filled = true;
				Grid[column][i].color = tok_color;
				Grid[column][i].coin = c_coin;
				grid_count++;
				return;
			}
	}
}

bool GameCheck(uint16_t color_p){ //checks for winnner
	for( int i = 6; i >= 0; i-- ) {
		for( int j = 5; j >= 1; j-- ) {
			
			if(Grid[i][j].color == color_p &&	Grid[i-1][j-1].color == color_p &&	Grid[i-2][j-2].color == color_p &&	Grid[i-3][j-3].color == color_p )	{
				return true;
			}
			

			if(Grid[i][j].color == color_p && Grid[i][j-1].color == color_p &&	Grid[i][j-2].color == color_p && Grid[i][j-3].color == color_p )	{
				return true;
			}
					
			if(Grid[i][j].color == color_p && Grid[i-1][j].color == color_p && Grid[i-2][j].color == color_p && Grid[i-3][j].color == color_p )	{	
				return true;
			}
					
			if(Grid[i][j].color == color_p && Grid[i-1][j+1].color == color_p && Grid[i-2][j+2].color == color_p &&	Grid[i-3][j+3].color == color_p )	{
				return true;
			}
						
			if ( Grid[i][j].color == color_p && Grid[i][j+1].color == color_p && Grid[i][j+2].color == color_p && Grid[i][j+3].color == color_p )	{
				return true;
			}
		}
		
	}
	return false;
}

void background(void){
  flag = 1; // semaphore

	blue.changeDistance(my.Distance());		//continue to change the distance of player token
	blue.currentCol(my.currentColumn());	//make sure to add column value
	tokenSelect = (GPIO_PORTE_DATA_R & 0x01);
	if(tokenSelect == true) {							//if PE0 was pressed place token in that column and check to see if player won
		playerWin = GameCheck(blue_c);
	}
	if(grid_count == 42 && playerWin == false) {	//if player didn't win and the board is full game is draw
		drawFlag = true;
	}
}

void ComputerPlay() {
	uint32_t c_col =(Random32()>>24)%7;
	while(Grid[c_col][0].filled == true) {
		c_col =(Random32()>>24)%7;
	}
	placeToken(c_col, red_c, redChip);
	compWin = GameCheck(red_c);
}


void IntroScreen(void){
	ST7735_FillScreen(0); //clear screen
	ST7735_SetCursor(0, 0);
	ST7735_OutString("ENGLISH PE0 BUTTON\nSPANISH PE1 BUTTON");
	int PE0 = 0;
	int PE1 = 1;
	while(1){
		PE0 = GPIO_PORTE_DATA_R &= 0x01;
		PE1 = GPIO_PORTE_DATA_R &= 0x02;
		
		if(PE0){
			ST7735_FillScreen(0); //clear screen
		  ST7735_OutString("ENGLISH\n");
			delay();
			ST7735_FillScreen(0); //clear screen
				break;
		}
		if(PE1){
			ST7735_FillScreen(0); //clear screen
			ST7735_OutString("SPANISH\n");
			spanishFlag = true;
			delay();
			delay();
			ST7735_FillScreen(0); //clear screen
			break;
		}
			
		
		
	}
		if(spanishFlag){
			ST7735_OutString("CONECTAR CUATRO");
			ST7735_DrawBitmap(5, 160, IntroScreenPic , 115,115);
		}
		else{
			ST7735_DrawBitmap(5, 160, IntroScreenPic , 115,115);
			ST7735_OutString("CONNECT FOUR");
		}
		delay();
		delay();
		ST7735_FillScreen(0); //clear screen

}
void congratsScreen(void){
	ST7735_DrawBitmap(5, 160, congrats , 115,115);
	congratsScreenSong();
	delay();
	delay();
	delay();
	delay();
	delay();
	delay();
	delay();
	
}
int main(void){
	
	
	DisableInterrupts();
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
	SysTick_Init(8000000);	//sample at 10Hz or 0.1s
  ST7735_InitR(INITR_REDTAB); 
  ADC_Init();        // turn on ADC, PD2, set channel to 5
  PortF_Init();
	PortE_Init();
  Random_Init(1);
  Output_Init();
	Sound_Init();
  Timer1_Init(&background,1600000); // 50 Hz
  //Timer1_Init(&clock,80000000); // 1 Hz
  EnableInterrupts();

  while(1){
		IntroScreen(); //so it can restart once the game is over
		ST7735_DrawBitmap(5,160,grid,115,100);
		GameInit();
		while(playerWin == false && compWin == false && drawFlag == false) { //play while no one has won or drawn
			while(flag==0){};		//semaphore
			flag = 0;	
			if(tokenSelect == true) { //if PE0 was pressed draw game and produce sound
				tokenSelect = false;
				placeToken(blue.getCol(), blue_c, blueChip);
				delay();
				//if PE0 was pressed place token in that column and check to see if player won
				playerWin = GameCheck(blue_c);
				if(grid_count == 42 && playerWin == false) {	//if player didn't win and the board is full game is draw
					drawFlag = true;
				}
				GameDraw();
				Sound_DropToken();
				if(playerWin == false && drawFlag == false){	//if the player does not win disable interrupts while the computer plays its turn
					DisableInterrupts();
					ComputerPlay();
					EnableInterrupts();
				}
			}
			GameDraw();							//still continue to draw the game
		}
		//write function for ending screen
		if(playerWin == true) {
			//ST7735_FillScreen(0);
			ST7735_SetCursor(0,3);
			if(spanishFlag){
				ST7735_OutString("BUEN TRABAJO,GANASTE!\n");
			}
				else{
			ST7735_OutString("GOOD JOB, YOU WON!\n");}
			Player_Score++;
			delay();
			delay();
			congratsScreen();
			
		} else if(compWin == true) {
			//ST7735_FillScreen(0);
			ST7735_SetCursor(0,3);
			if(spanishFlag){
				ST7735_OutString("AWW,PIERDES A MI!");
			}
				else{
			ST7735_OutString("AWW, YOU LOST TO ME!\n");}
			Comp_Score++;
			delay();
			delay();
		} else if(drawFlag == true) {
			//ST7735_FillScreen(0);
			ST7735_SetCursor(0,3);
			ST7735_OutString("OHH, IT'S A TIE!\n");
			delay();
			delay();
		}

  }

}



void SysTick_Handler(void){ // every sample
    //*** students write this ******
// should call ADC_In() and Sensor.Save
  GPIO_PORTF_DATA_R ^= 0x0E;		//toggle LED heartbeat
  my.Save(ADC_In());
	
	
}

