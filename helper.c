#include "helper.h"
#include "CU_TM4C123.h"

#include <string.h>

#define SETUPPORT   GPIOA
#define DATAPORT    GPIOB

#define GO_HIGH     0x0
#define TRANSITION  0x1
#define STAY_HIGH   0x2
#define GO_LOW      0x3
#define STAY_LOW    0x4

#define PRESCALE_VALUE              199UL
#define SYSTEM_CLOCK_FREQUENCY      16000000UL
#define MAX_SCROLL_LEVEL            7
#define MIN_SCROLL_LEVEL            0

const char CHARACTER_MAP[][8] = {
    // go_high state
    { 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 }, // top of go high
    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 }, // bot of go high
    // stay_high state
    { 0x1f, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 },        // top of stay high
    // go_low state
    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f }, // bot of go low
    // stay_low
    { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1f }         // bot of stay low
};


//const uint32_t SCROLL_LEVELS[] = { 7500, 5000, 2500, 1000, 500, 250, 10, 5, 2 };
const uint32_t SCROLL_LEVELS[] = { 2, 10, 50, 250, 750, 2500, 10000 };

// Global Variables
int current_scroll_level = 5;
uint32_t dummy_read;

int current_ddgram_pos = 0;
int previous_state = 0;
int current_state = 0;

// Initialization Functons

void InitializeGPIOPorts(void) {
    // Enable GPIOA, GPIOB, GPIOF.
    SYSCTL->RCGCGPIO |= (0x1UL<<0); // A
    SYSCTL->RCGCGPIO |= (0x1UL<<1); // B
    SYSCTL->RCGCGPIO |= (0x1UL<<5); // F
    
    // Do a dummy read to insert a few cycles after enabling the peripherals.
    dummy_read = SYSCTL->RCGCGPIO;

    // Setting up GPIOA pins 5, 4, 3, 2.
    // PA6 for the input voltage of the oscilloscope.
    SETUPPORT->DIR &= ~(0x1UL<<7);          // Set the direction as an input.
    SETUPPORT->DEN |=  (0x1UL<<7);          // Digital enable.

    // Setting up instruction pins; direction = output and digital enabled
    SETUPPORT->DIR |=  (0x1UL<<4);          // PA4 -> RS.
    SETUPPORT->DEN |=  (0x1UL<<4);
    SETUPPORT->DIR |=  (0x1UL<<3);          // PA3 -> R/W.
    SETUPPORT->DEN |=  (0x1UL<<3);
    SETUPPORT->DIR |=  (0x1UL<<2);          // PA2 -> E.
    SETUPPORT->DEN |=  (0x1UL<<2);

    // Setting up GPIOB pins 0-7 for data; direction output and digital enable.
    DATAPORT->DIR |= (0xFFUL);
    DATAPORT->DEN |= (0xFFUL);

    // Setting up GPIOF pins
    GPIOF->LOCK = 0x4C4F434B;               // Unlock GPIOF
    *((uint32_t *) &GPIOF->CR) = 0x1F;      // Enable write option to PUR for PF0.

    // Configure PF4 for button one.
    GPIOF->DIR &= ~(0x1UL<<4);              // configure port as input
    GPIOF->DEN |=  (0x1UL<<4);              // digital function
    GPIOF->PUR |=  (0x1UL<<4);              // pull up resistor
    GPIOF->IS  &= ~(0x1UL<<4);              // level sensitive interrupt
    GPIOF->IBE &= ~(0x1UL<<4);              // single edge interrupt
    GPIOF->IEV |=  (0x1UL<<4);              // rising edge interrupt
    GPIOF->IM  |=  (0x1UL<<4);              // Unmask GPIOF Pin 4

    // Configure PF0 for button two.
    GPIOF->DIR &= ~(0x1UL<<0);              // configure port as input
    GPIOF->DEN |=  (0x1UL<<0);              // digital function
    GPIOF->PUR |=  (0x1UL<<0);              // pull up resistor

    GPIOF->AMSEL &= ~(0x1UL<<0);
    GPIOF->PCTL  &= ~(0x1UL<<0);
    GPIOF->AFSEL &= ~(0x1UL<<0);
    GPIOF->IS    &= ~(0x1UL<<0);            // level sensitive interrupt
    GPIOF->IBE   &= ~(0x1UL<<0);            // single edge interrupt
    GPIOF->IEV   |=  (0x1UL<<0);            // rising edge interrupt
    GPIOF->IM    |=  (0x1UL<<0);            // Unmask GPIOF Pin 0   
}

void InitializeTimer0AForDelay(void) {   
    SYSCTL->RCGCTIMER |= (0x1UL<<0);        // Enable GPTM TIMER0
    TIMER0->CTL &= ~(0x1UL<<0);             // Disable TIMER0A for configuration
    TIMER0->CFG = (0x4UL);                  // Configure TIMER0 to be independent 16 bit timer
    TIMER0->TAMR = (0x2UL);                 // Set TIMERA to periodic mode. Default direction is to count down.
    TIMER0->TAPR = (19UL);                  // Setup Prescale value of 20-1=19
    TIMER0->TAILR = (79UL);                 // Setup interval of 80-1=79
    TIMER0->CTL |= (0x1UL<<0);              // Enable TIMERA
}

void InitializeTimer0BForScroll(void) {  
    NVIC->ICER[0] = (0x1UL<<20);
    SYSCTL->RCGCTIMER |= (0x1UL<<0);        // Enable GPTM TIMER0
    TIMER0->CTL &= ~(0x1UL<<8);             // Disable TIMER0B for configuration
    TIMER0->CFG = (0x4UL);                  // Configure TIMER0 to be independent 16 bit timer
    TIMER0->TBMR = (0x2UL);                 // Set TIMER0B to periodic mode. Default direction is to count down.
    TIMER0->TBPR = (399UL);                 // Setup Prescale value of 400-1=399
    TIMER0->TBILR = (99999UL);              // Setup interval of 20000-1=19999

    TIMER0->IMR |= (0x1UL<<8);              // Arm TIMER0B interrupt
}

void InitializeLCD(void) {
    // Clearing up the data port with 0
    DATAPORT->DATA  &= (0x0UL);
    SETUPPORT->DATA &= (0x0UL);
    
    SETUPPORT->DATA &= ~(0x1UL<<2);     // Enable set to 0.
    DelayMs(15);                        // Delay greater than 15ms.
    SendCommand(0x30UL);                // Wake up Command.
    DelayMs(5);                         // Delay greater than 4.1ms.
    SendCommand(0x30UL);                // Wake up Command.
    DelayMs(1);                         // Delay greater than 100us.
    SendCommand(0x30UL);                // Wake up Command.
    SendCommand(0x38UL);                // Function Set -> 8x2.
    SendCommand(0x1UL);                 // Clear Display.
    SendCommand(0x2UL);                 // Return Home.
    SendCommand(0xFUL);                 // Display ON, cursor on, cursor position on.
    SendCommand(0x6UL);                 // Entry Mode Set -> No shift, move cursor right.
}

void InitializeNVIC(void) {
    // set priority = 4
    NVIC->IP[20] = 4<<5;            // recall IRQ # = 20
    NVIC->ICPR[0] = (0x1UL<<20);    // clear pending bit … just to be safe
    NVIC->ISER[0] = (0x1UL<<20);    // enable interrupt at NVIC
    
    // Initialize GPIOF interrupt handler
    NVIC->IP[30] = 4<<5;            // Set priority = 4
    NVIC->ICPR[0] = (0x1UL<<30);    // Clear pending bit
    NVIC->ISER[0] = (0x1UL<<30);    // Enable interrupt at NVIC table
}

void InitializeSpecialCharacters(void) {
    int i, j;
    SendCommand(0x40); // Starting address of CGRAM.

    for (i=0; i<5; i++) {
        for (j=0; j<8; j++) {
            SendData(CHARACTER_MAP[i][j]);
        }
    }
}

// Utility Functions

void SendCommand (uint32_t instruction) {
    DATAPORT->DATA   = instruction;
    SETUPPORT->DATA &= ~(0x1UL<<4);     // RS = LOW
    SETUPPORT->DATA &= ~(0x1UL<<3);     // R/W = LOW
    SETUPPORT->DATA |= (0x1UL<<2);      // E = HIGH
    DelayMs(1);
    SETUPPORT->DATA &= ~(0x1UL<<2);     // E = LOW
}

void SendData(char data) {
    DATAPORT->DATA   =  (data);
    SETUPPORT->DATA |=  (0x1UL<<4);     // RS = HIGH
    SETUPPORT->DATA &= ~(0x1UL<<3);     // R/W = LOW
    SETUPPORT->DATA |=  (0x1UL<<2);     // E = HIGH
    DelayMs(1);
    SETUPPORT->DATA &= ~(0x1UL<<2);     // E =LOW
}

void writeString(char *string) {
    int i;
    for (i=0; i<strlen(string); i++) {
        SendData(string[i]);
    }
}

void DelayMs(uint32_t delayTimeInMs) {
    uint32_t zeroPointOneMs = 0;
    TIMER0->ICR = (0x0UL);
    while (delayTimeInMs*10 > zeroPointOneMs) {
        if (TIMER0->RIS & 0x1UL) {
            zeroPointOneMs++;
            TIMER0->ICR |= (0x1UL);
        }
    }   
}

void IncreaseFrequency() {    
    if (current_scroll_level < MAX_SCROLL_LEVEL-1) {
        TIMER0->CTL &= ~(0x1UL<<8); // Disable TIMER0B for configuration.
        TIMER0->IMR &= ~(0x1UL<<8); // Disarm TIMER0B interrupt.
            
        current_scroll_level++;

        // Calculate the new reload counter for TIMER0B interrupt.
        TIMER0->TBILR = ((SCROLL_LEVELS[current_scroll_level] * SYSTEM_CLOCK_FREQUENCY/(PRESCALE_VALUE+1)) - 1);
        TIMER0->IMR |= (0x1UL<<8);  // Arm TIMER0B interrupt.
        TIMER0->CTL |= (0x1UL<<8);  // Enable TIMER0B for configuration.
        }
}

void DecreaseFrequency(void) {
    if (current_scroll_level > MIN_SCROLL_LEVEL) {
        TIMER0->CTL &= ~(0x1UL<<8); // Disable TIMER0B for configuration.
        TIMER0->IMR &= ~(0x1UL<<8); // Disarm TIMER0B interrupt.

        current_scroll_level--;
            
        // Calculate the new reload counter value for TIMER0B interrupt.
        TIMER0->TBILR = ((SCROLL_LEVELS[current_scroll_level] * SYSTEM_CLOCK_FREQUENCY/(PRESCALE_VALUE+1)) - 1);
        TIMER0->IMR |= (0x1UL<<8);  // Arm TIMER0B interrupt.
        TIMER0->CTL |= (0x1UL<<8);  // Enable TIMER0B for configuration.
        }
}

void TIMER0B_Handler(void) { 
    current_state = (SETUPPORT->DATA &= (0x1UL<<7));    // Active low.
    
    if (current_state) {           // button one was pressed.
        if (previous_state) {      // previous state was a high.
            PrintStayHigh();
            SendCommand(0x18);
        } else {                  // previous state was a low.
            PrintGoHigh();
            SendCommand(0x18);
        }
    } else {                      // button one is not pressed.
        if (previous_state) {      // previous state was a high.
            PrintGoLow();
            SendCommand(0x18);
        } else {                  // previous state was a low.
            PrintStayLow();
            SendCommand(0x18);
        }
    }
    previous_state = current_state;
    
    // Clear interrupt at GPTM ... de-assert IRQ 19 signal.
    TIMER0->ICR = (0x1UL<<8);
    
    // Clear pending bit in NVIC
    NVIC->ICPR[0] = (0x1UL<<20);
}

void ScrollDisplay(void) {
        SendCommand(0x18UL);
}

void GPIOF_Handler(void) {
    if (GPIOF->RIS & (0x1UL<<4)) {
        IncreaseFrequency();
        GPIOF->ICR |= (0x1UL<<4); // Clear interrupt at 4.  
    } else if (GPIOF->RIS & (0x1UL<<0)) {
        DecreaseFrequency();
        GPIOF->ICR |= (0x1UL<<0); // Clear interrupt at 0.
    }

    NVIC->ICPR[0] = 0x1UL<<30; // Clear pending bit.
    NVIC->ICPR[0] = 0x1UL<<20;
}

// Functions to print special characters for the 1 bit osciliscope

void PrintGoHigh(void) {
    SendCommand(0x80 + (current_ddgram_pos%=40));
    SendData(GO_HIGH);
    SendCommand(0xC0 + (current_ddgram_pos%=40));
    SendData(TRANSITION);
    current_ddgram_pos++;
}

void PrintStayHigh(void) {
    SendCommand(0x80 + (current_ddgram_pos%=40));
    SendData(STAY_HIGH);
    SendCommand(0xC0 + (current_ddgram_pos%=40));
    SendData(' '); // Need to put a blank to erase previous data in this address.
    current_ddgram_pos++;
}

void PrintGoLow(void) {
    SendCommand(0x80 + (current_ddgram_pos%=40));
    SendData(TRANSITION);
    SendCommand(0xC0 + (current_ddgram_pos%=40));
    SendData(GO_LOW);
    current_ddgram_pos++;
}

void PrintStayLow(void) {
    SendCommand(0x80 + (current_ddgram_pos%=40));
    SendData(' '); // Need to put a blank to erase previous data in this address.
    SendCommand(0xC0 + (current_ddgram_pos%=40));
    SendData(STAY_LOW);
    current_ddgram_pos++;
}
