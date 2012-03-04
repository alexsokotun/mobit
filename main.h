#ifndef __MAIN_H__
#define __MAIN_H__

// Режим обмена
#define _MASTER_

// Частотный канал
#define RF_CHANNEL    35

#include <iom16.h>
#include <inavr.h>

// Переопределение типов
typedef unsigned char u08; 		// 	8-bit
typedef unsigned int u16; 		// 	16-bit
typedef unsigned long u32; 		// 	32-bit
typedef unsigned long long u64; // 	64-bit
typedef signed char s08; 		// 	8-bit
typedef signed int s16; 		//	16-bit
typedef signed long s32; 		//	32-bit
typedef signed long long s64; 	//	64-bit

#include "uart.h"
#include "timer.h"
#include "radio.h"
#include "spi.h"
#include "crc.h"
#include "nmea.h"


#define PIN_SET(port,bit) {port |= (1 << bit);  __no_operation();}
#define PIN_RESET(port,bit)  {port &= ~(1 << bit);  __no_operation();}

#define LED_On()      PIN_RESET(PORTD,2)
#define LED_Off()     PIN_SET(PORTD,2)

#endif
