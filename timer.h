#ifndef __TIMER_H__
#define __TIMER_H__

#define ENABLE_T2_INTERRUPT()   {TIMSK |= (1<<OCIE2);}
#define DISABLE_T2_INTERRUPT()   {TIMSK &= ~(1<<OCIE2);}

// Инициализация таймера
void Timer_Init (void);

#endif
