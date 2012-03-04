#include "main.h"

// SPI
void SPIInit(void) {
  /* Set MOSI and SCK output, all others input */
  DDRB |= (1<</*DD_MOSI*/ PORTB5) | (1<< /*DD_SCK*/ PORTB7);
  /* Enable SPI, Master, set clock rate  */
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPHA);
}
