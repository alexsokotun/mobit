#ifndef UART_H
#define UART_H

#define UART_EMPTY()   (UCSRA & (1<<UDRE))
void USART_Init(void);

#endif
