#ifndef __RADIO_H__
#define __RADIO_H__

// RF(EM198810) registers

#define		RF_TX_RX_CONTROL	7

#define		RF_TRAN_MODE		0x0100	/* режим передачи */
#define		RF_RECV_MODE		0x0080	/* режим приёма */
#define		RF_IDLE_MODE		0x0000	/* IDLE режим */

#define		RF_TX_FIFO_REG		80

#define		RF_FIFO_RD_PTR		82
#define		RF_CLEAR_TXFIFO		0x8000	/* clear TX FIFO point */
#define		RF_CLEAR_RXFIFO		0x0080	/* clear RX FIFO point */


// Точки в FIFO
#define SPI_BUFFER_SIZE 19
#define FIFO_SET_INDEX  0
#define FIFO_GET_INDEX  1
#define FIFO_DATA_START 2
#define FIFO_CS_INDEX  SPI_BUFFER_SIZE-1

#define CS_INITIAL      0xFF

// Период ожидания WD для радио - 50 мс
#define WD_Treshold   50

// Предупреждение другой стороны об отсутствии приема за 10мс до перезагрузки
#define WD_Rx_Treshold  10

// Время посылки предупреждения
#define WD_Pre_Treshold WD_Treshold-WD_Rx_Treshold

// Radio machine States
#define Radio_Reset   0
#define Radio_Reset1  1
#define Radio_Reset2  2
#define Radio_Reset3  3
#define Radio_Reset4  4
#define TX_Timer      5
#define TX_Pkt        6
#define RX_Pkt        7


//------------------------------------------------------------------------------------------------
//                                  RESETN Pin Configuration
//------------------------------------------------------------------------------------------------

#define LAN_DRIVE_RESETN_PIN_HIGH()         PIN_SET(PORTB,1)
#define LAN_DRIVE_RESETN_PIN_LOW()          PIN_RESET(PORTB,1)
#define CONFIG_RESETN_PIN_AS_OUTPUT()       PIN_SET(DDRB,1)


//------------------------------------------------------------------------------------------------
//                                     SPI Configuration
//------------------------------------------------------------------------------------------------

// SS Pin Configuration
#define LAN_SPI_SET_SS_ON()			PIN_RESET(PORTB,4)
#define LAN_SPI_SET_SS_OFF()			PIN_SET(PORTB,4)
#define LAN_SPI_SS_IS_OFF()			(PINB & (1 << 4))
#define LAN_SPI_CONFIG_SS_PIN_AS_OUTPUT()	PIN_SET(DDRB,4)

//------------------------------------------------------------------------------------------------
//                                     PKT
//------------------------------------------------------------------------------------------------

#define ENABLE_PKT_INTERRUPT  { PIN_SET(GICR,INT2); }
#define DISABLE_PKT_INTERRUPT { PIN_RESET(GICR,INT2);}
#define RESET_PKT_INTERRUPT_FLAG { PIN_SET(GIFR,INT2);}


//------------------------------------------------------------------------------------------------
//                                     Variables
//------------------------------------------------------------------------------------------------

extern u08 Timer_Cnt; // Счетчик мс
extern u08 Radio_Active; // Индикатор наличия связи

// Указатели на данные
extern u08 TX_SET_FIFO_Ptr;
extern u08 RX_GET_FIFO_Ptr;

//! Номер RX buffer for SPI [0/1].
extern u08 SPI_Rx_Cur_Buffer;
//! RX buffer for SPI.
extern u08 * SPI_RxBuffer;
//! TX buffer for UART.
extern u08 * UART_Tx_Buffer;

//! RX buffer for SPI.
extern u08 SPI_RxBuffer_0[SPI_BUFFER_SIZE];
//! RX buffer for SPI.
extern u08 SPI_RxBuffer_1[SPI_BUFFER_SIZE];
//! TX buffer for SPI.
extern u08 SPI_TxBuffer[SPI_BUFFER_SIZE];

//------------------------------------------------------------------------------------------------
//                                        Function Prototype
//------------------------------------------------------------------------------------------------


void Radio_Start (void);

#endif
