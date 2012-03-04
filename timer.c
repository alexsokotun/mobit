#include "main.h"

// Инициализация таймера
void Timer_Init (void)
{
  TCCR2=0x4B; // 0100 1011 Fast PWM F/32 autoreload
  ASSR=0; //fclk
  TCNT2=0;
  OCR2=250; // 8000000/32/250 = 1mc
}

