//*****************************************************************************
//
// week1.c - Initialize LCD to enable capability of writing on the LCD.
// Version 1.0: distributed with SYSC4906F15 Lab1
//
//*****************************************************************************

#include "CU_TM4C123.h"

#define SETUPPORT GPIOA
#define DATAPORT GPIOB

// Procedure for a delay function
void delayMs(uint32_t delayTimeInMs);
// Procedure to send instructions to the LCD
void command(uint32_t instruction);
// Procedure to initialize the LCD
void initializeLCD();
// Procedure to send character data to the LCD
void writeCharacter(uint32_t data);

int main(void)
{
	uint32_t dummyReadValue;
	
	// Enable GPIO Port A and B that is mapped to a Tiva Launchpad pin
	SYSCTL->RCGCGPIO |= (0x1UL<<0); 
	SYSCTL->RCGCGPIO |= (0x1UL<<1); 
		
	// Do a dummy read to insert a few cycles after enabling the peripherals.
	dummyReadValue = SYSCTL->RCGCGPIO;

	// Setting up GPIOA pins 4, 3, 2. Direction output and enable.
	SETUPPORT->DIR |= (0x1UL<<4); // PA4 -> RS
	SETUPPORT->DEN |= (0x1UL<<4);
	SETUPPORT->DIR |= (0x1UL<<3); // PA3 -> R/W
	SETUPPORT->DEN |= (0x1UL<<3);
	SETUPPORT->DIR |= (0x1UL<<2);	// PA2 -> E
	SETUPPORT->DEN |= (0x1UL<<2);
	
	// Setting up GPIOB pins 0-7 for data. Direction output and enable	
	DATAPORT->DIR |= (0xFFUL);
	DATAPORT->DEN |= (0xFFUL);
			
	// Enable GPTM timer 0
	SYSCTL->RCGCTIMER |= (0x1UL<<0);
	// Disable timer A for configuration
	TIMER0->CTL &= ~(0x1UL<<0);
	// Configure timer0 to be independent 16 bit timer.
  TIMER0->CFG = (0x4UL);
	// Only need to change settings on timer A registers. Set timer A to periodic mode.
	// Default direction is to count down.
	TIMER0->TAMR = (0x2UL);
	// Setup Prescale value of 20-1=19
	TIMER0->TAPR = (0x19UL);
	// Setup interval of 80-1=79
	TIMER0->TAILR = (79UL);
	// Enable timer A
	TIMER0->CTL |= (0x1UL<<0);

	initializeLCD();
	
	delayMs(1000);
	writeCharacter('A');
	//delayMs(1000);
	writeCharacter('M');
	//delayMs(1000);
	writeCharacter('E');
	//delayMs(1000);
	writeCharacter('N');
	//delayMs(1000);
	writeCharacter('T');
	//delayMs(1000);
	writeCharacter('E');
	//delayMs(1000);
	writeCharacter(' ');
	//delayMs(1000);
	writeCharacter('#');
	//delayMs(1000);
	writeCharacter('1');
	//delayMs(1000);
	writeCharacter(' ');
	//delayMs(1000);
	writeCharacter('T');
	//delayMs(1000);
	writeCharacter('A');
	//delayMs(1000);
	
	while(1){
	}
}

void delayMs(uint32_t delayTimeInMs){
	uint32_t zeroPointOneMs = 0;
	
	while (delayTimeInMs*10 > zeroPointOneMs){
		if (TIMER0->RIS & 0x1UL){
			zeroPointOneMs++;
			TIMER0->ICR |= (0x1UL);
		}
	}	
}

void command (uint32_t instruction){
	DATAPORT->DATA = instruction;
	SETUPPORT->DATA &= ~(0x1UL<<4); // RS = LOW
	SETUPPORT->DATA &= ~(0x1UL<<3); // R/W = LOW
	SETUPPORT->DATA |= (0x1UL<<2); // E = HIGH
	delayMs(1);
	SETUPPORT->DATA &= ~(0x1UL<<2); // E = LOW
}

void initializeLCD(){
	// Clearing up the data port with 0
	DATAPORT->DATA &= (0x0UL);
	SETUPPORT->DATA &= (0x0UL);
	
	// Enable set to 0
	SETUPPORT->DATA &= ~(0x1UL<<2);
	
	// Delay greater than 15ms
	delayMs(15);
	// Wake up Command
	command(0x30UL);	
	// Delay greater than 4.1ms
	delayMs(5);
	// Wake up Command
	command(0x30UL);	
	// Delay greater than 100us
	delayMs(1);
	// Wake up Command
	command(0x30UL);
	
	command(0x38UL); // Function Set -> 8x2
	command(0x1UL);	// Clear Display
	command(0x2UL);	// Return Home
	command(0xFUL); // Display ON, cursor on, cursor position on
	command(0x6UL); // Entry Mode Set -> No shift, move cursor right
}

void writeCharacter(uint32_t data){
	DATAPORT->DATA = (data);
	SETUPPORT->DATA |= (0x1UL<<4); // RS = HIGH
	SETUPPORT->DATA &= ~(0x1UL<<3); // R/W = LOW
	SETUPPORT->DATA |= (0x1UL<<2); // E = HIGH
	delayMs(1);
	SETUPPORT->DATA &= ~(0x1UL<<2); // E =LOW
}
