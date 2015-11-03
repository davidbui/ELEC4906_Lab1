#include "CU_TM4C123.h"

void GPIO_Init(void);
void PWMTimer_Init(void);
void DelayTimer_Init(void);
void DelayMs(float delayTimeInMs);
void NVIC_Init(void);

void EnablePWM(void);
void DisablePWM(void);
void StartTx(void);
void Tx0(void);
void Tx1(void);
void EndTx(void);

int main(void){
	
	GPIO_Init();
	PWMTimer_Init();
	DelayTimer_Init();
	NVIC_Init();
	
	DelayMs(1000);
	
	StartTx();
	
	//Address
	Tx0();	
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	
	Tx1();
	Tx1();
	Tx1();
	Tx1();
	Tx0();
	Tx1();
	Tx1();
	Tx1();
	
	// Data
	Tx1();
	Tx1();
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	
	Tx0();
	Tx0();
	Tx1();
	Tx1();
	Tx1();
	Tx1();
	Tx1();
	Tx1();
	
	EndTx();
	
	DelayMs(1000);
	
	// RED

StartTx();
	
	//Address
	Tx0();	
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	
	Tx1();
	Tx1();
	Tx1();
	Tx1();
	Tx0();
	Tx1();
	Tx1();
	Tx1();
	
	// Data
	Tx0();
	Tx0();
	Tx1();
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	Tx0();
	
	Tx1();
	Tx1();
	Tx0();
	Tx1();
	Tx1();
	Tx1();
	Tx1();
	Tx1();
	
	EndTx();
}

void EnablePWM(void){
//	GPIOC->DEN |= (1UL<<4);
	WTIMER0->CTL |= (1UL << 0);
}


void DisablePWM(void){	
	WTIMER0->CTL &= ~(1UL << 0);
//	GPIOC->DEN &= ~(1UL << 4);
}

void StartTx(void){
	EnablePWM();
	DelayMs(9);
	DisablePWM();
	DelayMs(4.5);
}

void Tx0(void){
	EnablePWM();
	DelayMs(0.56);
	DisablePWM();
	DelayMs(0.56);
}

void Tx1(void){
	EnablePWM();
	DelayMs(0.56);
	DisablePWM();
	DelayMs(1.69);
}

void EndTx(void){
	EnablePWM();
	DelayMs(0.56);
	DisablePWM();
}

void GPIO_Init(void){
	
	uint32_t dummy;
	
	// Enable GPIOC for PWM
	SYSCTL->RCGCGPIO |= (1UL << 2);
	
	// Do a dummy read to insert a few cycles after enabling the peripheral
	dummy = SYSCTL->RCGCGPIO;
	
	// Enable PC4 for PWM output
	GPIOC->DEN |= (1UL<<4);
	GPIOC->PCTL |= (7UL<<16);
	GPIOC->AFSEL |= (1UL<<4);
}

// Setup WTimer0A to be 38kHz PWM
void PWMTimer_Init(void){
	SYSCTL->RCGCWTIMER |= (1UL << 0); // WTimer0
	WTIMER0->CTL &= ~(1UL << 0);  // Disable timer (TnEN)
	WTIMER0->CFG |= 4UL;
	WTIMER0->TAMR |= (1UL << 3);  // Enable PWM
	WTIMER0->TAMR &= ~(1UL << 2); 
	WTIMER0->TAMR |= (1UL << 1);
	WTIMER0->CTL &= ~(1UL << 6);
	WTIMER0->TAILR = 420UL;	// 38kHz
	WTIMER0->TAMATCHR = 210UL; // 50% Duty cycle cus half dat
	
//	SYSCTL->RCGCTIMER |= (0x1UL<<0); // Enable GPTM TIMER0
//	TIMER0->CTL &= ~(0x1UL<<0);	// Disable TIMER0A for configuration
//  TIMER0->CFG = (0x4UL);	// Configure TIMER0 to be independent 16 bit timer
//	TIMER0->TAMR = (0x2UL); // Set TIMERA to periodic mode. Default direction is to count down.
//	TIMER0->TAPR = (10UL); // Setup Prescale value of 20-1=19
//	TIMER0->TAILR = (7UL);	// Setup interval of 8-1=7
//	TIMER0->CTL |= (0x1UL<<0);	// Enable TIMERA
//	
//	SYSCTL->RCGCTIMER |= (0x1UL<<0);        // Enable GPTM TIMER0.
//  TIMER0->CTL &= ~(0x1UL<<8);             // Disable TIMER0B for configuration.
//  TIMER0->CFG = (0x4UL);                  // Configure TIMER0 to be independent 16 bit timer.
//  TIMER0->TBMR = (0x2UL);                 // Set TIMER0B to periodic mode. Default direction is to count down.
//  TIMER0->TBPR = (399UL);                 // Setup Prescale value of 400-1=399.
//  TIMER0->TBILR = (19999UL);              // Setup interval of 20000-1=19999.
	
	//GPIOC->DATA &= ~(1UL << 4);
	//GPIOC->DEN &= ~(1UL<<4);
	//WTIMER0->CTL |= (1UL << 0);  // ENABLE timer (TnEN)
}

// Setup Timer0A to be 100kHz (period = 0.01ms)
void DelayTimer_Init(void){	
	SYSCTL->RCGCTIMER |= (0x1UL<<0); // Enable GPTM TIMER0
	TIMER0->CTL &= ~(0x1UL<<0);	// Disable TIMER0A for configuration
  TIMER0->CFG = (0x4UL);	// Configure TIMER0 to be independent 16 bit timer
	TIMER0->TAMR = (0x2UL); // Set TIMERA to periodic mode. Default direction is to count down.
	TIMER0->TAPR = (19UL); // Setup Prescale value of 20-1=19
	TIMER0->TAILR = (7UL);	// Setup interval of 8-1=7
	TIMER0->CTL |= (0x1UL<<0);	// Enable TIMERA
}

void DelayMs(float delayTimeInMs) {
	uint32_t zeroPointZeroOneMs = 0;
	TIMER0->ICR = (0x0UL);
	while (delayTimeInMs*100 > zeroPointZeroOneMs){
		if (TIMER0->RIS & 0x1UL){
			zeroPointZeroOneMs++;
			TIMER0->ICR |= (0x1UL);
		}
	}
}

void NVIC_Init(void){
//	// set priority = 4 (recall example details)
//	NVIC->IP[20] = 4<<5; // recall IRQ # = 20
//	// clear pending bit … just to be safe
//	NVIC->ICPR[0] = (0x1UL<<20);
//	// enable interrupt at NVIC
//	NVIC->ISER[0] = (0x1UL<<20);
}
