#include "main.h"

// Инициализация ножек
void IO_Init(void)
{
	PORTA = 0;
	DDRA = 0x0;
	

        PORTD = 0xF0;	
	DDRD = 0x06;

        PORTB = 0x12; // 00010010
	DDRB = 0xB2; // 10110010

	PORTC = 0;
	DDRC = 0x0;
}

// Главная функция
void main (void)
{
	
	//Initialisation
	
        IO_Init();
        SW_Init();
        USART_Init();
        SPIInit();
        Timer_Init();
        Radio_Start();
        // поехали... :();

	while (1)
        {
          if (Radio_Active)
            {LED_On();}
          else
            {LED_Off();}
	
        }
	// приехали!
}
