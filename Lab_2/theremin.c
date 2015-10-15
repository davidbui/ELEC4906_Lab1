#include "CU_TM4C123.h"
void GPIO_init(void);
void ADC1SS3_Handler(void);
void ADC0SS3_Handler(void);
void InitializeTimer0BForScroll(void);
void InitializeNVIC(void);

volatile static uint32_t adc0Result = 0;
volatile static uint32_t adc1Result = 0;

#define C 30578
#define D 27242
#define E 24270
#define F 22908
#define G 20408
#define A 18182
#define B 16198
#define C2 15289

unsigned int scale[] = { C, D, E, F, G, A, B, C2 };
unsigned int twinkle_twinkle_little_star_notes[] = 
{
	C, C, G, G, A, A, G,
	F, F, E, E, D, D, C,
	G, G, F, F, E, E, D,
	G, G, F, F, E, E, D,
	C, C, G, G, A, A, G,
	F, F, E, E, D, D, C
};

int twinkle_twinkle_little_star_timings[] =
{
	1, 1, 1, 1, 1, 1, 2
};

void delay(int n)
{
	unsigned int i;
	for (i=0; i<0x9ffff*n; i++);
}
	
void play_twinkle_twinkle_little_star()
{
	int i;
	for (i=0; i<42; i++)
	{
		
			WTIMER0->TAILR = twinkle_twinkle_little_star_notes[i];
			delay(twinkle_twinkle_little_star_timings[i % 7]);
		  WTIMER0->TAILR = 0;
		  delay(1);
	}
}

int main(void)
{
	GPIO_init();
	
	while(1)
	{
		//play_twinkle_twinkle_little_star();
	}
	

}
void GPIO_init(void)
{
	uint32_t dummy;
	
  //Enable GPIO E&F
  SYSCTL->RCGCGPIO |= (1UL << 4);
  SYSCTL->RCGCGPIO |= (1UL << 5);
	
	// Enable GPIOC for pwm
	SYSCTL->RCGCGPIO |= (1UL << 2);	
	
	// Enable ADC0 and ADC1
	SYSCTL->RCGCADC |= (1UL << 0);  //enable ADC0
	SYSCTL->RCGCADC |= (1UL << 1);  //enable ADC1
	
	// Do a dummy read to insert a few cycles after enabling the peripheral.
	dummy = SYSCTL->RCGCGPIO;
	
	// Enable PC4 for PWM output
	GPIOC->DEN |= (1UL<<4);
	GPIOC->PCTL |= (7UL<<16);
	GPIOC->AFSEL |= (1UL<<4);

  // Enable the GPIO pin for the LED (PF3).
	GPIOF->DIR |= 0x08;         // Set the direction as outputi
	GPIOF->DEN |= 0x08;         // Enable the GPIO pin for digital function
	GPIOF->AFSEL &= ~(1UL << 3);  //Disable Analog func
	GPIOF->DATA |= 0x08;   //Test LED
	
	// Enable PE3 for ADC IN 0
	GPIOE->DEN &= ~(1UL << 3);   //Disable Dig func
	GPIOE->AFSEL |= (1UL << 3);  // enable alternate function
	GPIOE->AMSEL |= (1UL << 3);  //PE3 ADC IN 0
	
	// Enable PE2 for ADC IN 1
	GPIOE->DEN &= ~(1UL << 2);   //Disable Dig func
	GPIOE->AFSEL |= (1UL << 2);  // enable alternate function
	GPIOE->AMSEL |= (1UL << 2);  //PE2 ADC IN 1
	
	InitializeTimer0BForScroll();
	InitializeNVIC();
  TIMER0->CTL |= (0x1UL<<8);  // Enable TIMER0B
	
	//Setup ADC1-SS3
  ADC1->ACTSS &= ~(1UL << 3);  //Disable SS3 for setup
  //ADC1->EMUX |= (0xF<<12);   //Always sample
  ADC1->EMUX &= ~(0xFUL<<12);   //Software trigger
  ADC1->SSMUX3 &= ~(0xFUL<<0);  //AIN0
  ADC1->SSCTL3 |= 0x6;    //END0 and IE0 for Interrupts
  ADC1->IM |= (1<<3);    //SS3 Interrupt Mask
  ADC1->ACTSS |= (1UL << 3);  //Enable SS3
  ADC1->ISC |= (1UL << 3);  //Clear Interrupt bit.
  NVIC_EnableIRQ(ADC1SS3_IRQn);
	
	//Setup ADC0-SS3
  ADC0->ACTSS &= ~(1UL << 3);  //Disable SS3 for setup
  //ADC1->EMUX |= (0xF<<12);   //Always sample
  ADC0->EMUX &= ~(0xFUL<<12);   //Software trigger
  ADC0->SSMUX3 |= (0x1UL);  //AIN1
  ADC0->SSCTL3 |= 0x6;    //END0 and IE0 for Interrupts
  ADC0->IM |= (1<<3);    //SS3 Interrupt Mask
  ADC0->ACTSS |= (1UL << 3);  //Enable SS3
  ADC0->ISC |= (1UL << 3);  //Clear Interrupt bit.
  NVIC_EnableIRQ(ADC0SS3_IRQn);
	
	// Enable Timer PWM
	// Timer0 Timer A
	SYSCTL->RCGCWTIMER |= (1UL << 0); // WTimer0
	WTIMER0->CTL &= ~(1UL << 0);  // Disable timer (TnEN)
	WTIMER0->CFG |= 4UL;
	WTIMER0->TAMR |= (1UL << 3);  // Enable PWM
	WTIMER0->TAMR &= ~(1UL << 2); 
	WTIMER0->TAMR |= (1UL << 1);
	WTIMER0->CTL |= (1UL << 6);
	WTIMER0->TAILR = 16364UL;	// 440Hz
	WTIMER0->TAMATCHR = 8182UL; // 50% Duty cycle cus half dat
	WTIMER0->CTL |= (1UL << 0);  // ENABLE timer (TnEN)
}

void InitializeTimer0BForScroll(void) {  
  NVIC->ICER[0] = (0x1UL<<20);
  SYSCTL->RCGCTIMER |= (0x1UL<<0);        // Enable GPTM TIMER0.
  TIMER0->CTL &= ~(0x1UL<<8);             // Disable TIMER0B for configuration.
  TIMER0->CFG = (0x4UL);                  // Configure TIMER0 to be independent 16 bit timer.
  TIMER0->TBMR = (0x2UL);                 // Set TIMER0B to periodic mode. Default direction is to count down.
  TIMER0->TBPR = (399UL);                 // Setup Prescale value of 400-1=399.
  TIMER0->TBILR = (19999UL);              // Setup interval of 20000-1=19999.

  TIMER0->IMR |= (0x1UL<<8);              // Arm TIMER0B interrupt.
}

void InitializeNVIC(void) {
  // set priority = 4
  NVIC->IP[20] = 4<<5;            // Recall IRQ # = 20.
  NVIC->ICPR[0] = (0x1UL<<20);    // Clear pending bit.
  NVIC->ISER[0] = (0x1UL<<20);    // Enable interrupt at NVIC.
}

void TIMER0B_Handler(void) {
		ADC1->PSSI |= (1UL<<3);
		ADC0->PSSI |= (1UL<<3);
	
    // Clear interrupt at GPTM
    TIMER0->ICR = (0x1UL<<8);    
    // Clear pending bit in NVIC
    NVIC->ICPR[0] = (0x1UL<<20);
}

void ADC1SS3_Handler(void)
{
	adc1Result = ADC1->SSFIFO3;
	adc1Result/=512;

	if (adc1Result > 6)
		WTIMER0->TAMATCHR = 8182UL; // 50% Duty cycle cus half dat
	else if(adc1Result > 5)
		WTIMER0->TAMATCHR = 4091UL; // 25% Duty cycle cus half dat
	else if(adc1Result > 4)
		WTIMER0->TAMATCHR = 2045UL; // 12% Duty cycle cus half dat
	else if(adc1Result > 3)
		WTIMER0->TAMATCHR = 1022UL; // 6% Duty cycle cus half dat
	else if(adc1Result > 2)
		WTIMER0->TAMATCHR = 511UL; // 3% Duty cycle cus half dat
	else if(adc1Result > 1)
		WTIMER0->TAMATCHR = 255UL; // 1.5% Duty cycle cus half dat
	else {
		WTIMER0->TAMATCHR = 0UL; // 0% Duty cycle cus half dat
	}
 
 ADC1->ISC |= (1UL << 3);  // Clear interrupt
}

void ADC0SS3_Handler(void)
{
	adc0Result = ADC0->SSFIFO3;
	adc0Result/=512;

//	if (adc0Result > 3) {
//		GPIOF->DATA |= (1UL<<3);
//	}
//	else {		
//		GPIOF->DATA &= ~(1UL<<3);
//	}
	
	if (adc0Result > 6)
		WTIMER0->TAILR = C;
	else if(adc0Result > 5)
		WTIMER0->TAILR = D;
	else if(adc0Result > 4)
		WTIMER0->TAILR = E;
	else if(adc0Result > 3)
		WTIMER0->TAILR = F;
	else if(adc0Result > 2)
		WTIMER0->TAILR = A;
	else if(adc0Result > 1)
		WTIMER0->TAILR = B;
	else {
		WTIMER0->TAILR = C2;
	}
 
 ADC0->ISC |= (1UL << 3);  // Clear interrupt
}
