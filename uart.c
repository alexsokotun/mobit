#include "main.h"

void USART_Init( void )
{
// 8 Mhz - 4800
UBRRH = 0;
UBRRL = 103;
/* Enable receiver and transmitter */
UCSRB = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN);
/* Set frame format: 8n1 */
UCSRC = (1<<URSEL)|(3<<UCSZ0);
}

// Байт пришел
#pragma vector = USART_RXC_vect
__interrupt void USART_RX_interrupt_handler(void)
{	
  u08 SPtr_, tmp_;
    // ------> Добавляем в очередь
      // Берем следующий индекс
      SPtr_=SPI_TxBuffer[FIFO_SET_INDEX];
      SPtr_++;
      if (SPtr_ >= SPI_BUFFER_SIZE-1)
        SPtr_ = FIFO_DATA_START;
      SPI_TxBuffer[FIFO_SET_INDEX]=SPtr_;
      // Берем байт из COM-порта
      tmp_=UDR;
      SPI_TxBuffer[SPtr_]=tmp_;
      PCK_End = (tmp_==0x0A ? 1 : 0);
}
