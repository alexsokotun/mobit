#include "main.h"

#define  TR24_Reg_Grp1_Len 11*4
const u08 TR24_Reg_Grp1[TR24_Reg_Grp1_Len] = {
 0, 48, 0x98, 0x20,
 0, 49, 0xFF, 0x0F,
 0, 50, 0xC8, 0x28,
 0, 51, 0xC8, 0x56,
 0, 52, 0x4E, 0xF6,
 0, 53, 0xF6, 0xF5,
 0, 54, 0x18, 0x5C,
 0, 55, 0xD6, 0x51,
 0, 56, 0x44, 0x04,
 0, 57, 0xE0, 0x00,
 0, 58, 0x00, 0x00
};

#define  TR24_Reg_Grp2_Len 24*4
const u08 TR24_Reg_Grp2[TR24_Reg_Grp2_Len] = {
 0, 0, 0x35, 0x4F,
 0, 2, 0x1F, 0x01,
 0, 4, 0x38, 0xD8,
 0, 5, 0x00, 0xA1,
 0, 7, 0x12, 0x4C,
 0, 8, 0x80, 0x08,
 0, 9, 0x21, 0x01,
 0, 10, 0x00, 0x04,
 0, 11, 0x40, 0x41,
 0, 12, 0x7E, 0x00,
 0, 13, 0x00, 0x00,
 0, 14, 0x16, 0x9D,
 0, 15, 0x80, 0x2F,
 0, 16, 0xB0, 0xF8,
 0, 18, 0xE0, 0x00,
 0, 19, 0xA1, 0x14,
 0, 20, 0x81, 0x91,
 0, 21, 0x69, 0x62,
 0, 22, 0x00, 0x02,
 0, 23, 0x00, 0x02,
 0, 24, 0xB1, 0x40,
 0, 25, 0x78, 0x0F,
 0, 26, 0x3F, 0x04,
 0, 28, 0x58, 0x00
};

// Канал
u08 LanRFPLLChNb;

u08 Timer_Cnt; // Счетчик мс
u08 Radio_State; // Переменная машины состояний
u08 WD_Cnt; // Счетчик ошибок
u08 WD_Rx_Time; // Счетчик времени по предупреждению принимающей стороны о перезагрузке
u08 Radio_Ready; // Флаг работы радио в режиме обмена
u08 Radio_Active; // Индикатор наличия связи

// Указатели на данные
u08 TX_SET_FIFO_Ptr;
u08 RX_GET_FIFO_Ptr;
u08 RX_CHK_FIFO_Ptr;

//! TX buffer for SPI.
u08 SPI_TxBuffer[SPI_BUFFER_SIZE];

//! Номер RX buffer for SPI [0/1].
u08 SPI_Rx_Cur_Buffer;
//! RX buffer for SPI.
u08 * SPI_RxBuffer;
//! RX buffer for SPI.
u08 * SPI_RxRdyBuffer;
//! TX buffer for UART.
u08 * UART_Tx_Buffer;

//! RX buffer for SPI.
u08 SPI_RxBuffer_0[SPI_BUFFER_SIZE];
//! RX buffer for SPI.
u08 SPI_RxBuffer_1[SPI_BUFFER_SIZE];


//------------------------------------------------------------------------------------------------
// 								Запуск радио
//------------------------------------------------------------------------------------------------
void Radio_Start (void)
{
  char i_;

  // activate INT2 on rasing edge
  PIN_SET(MCUCSR,ISC2);

  LanRFPLLChNb=RF_CHANNEL; // Канал

  // configure SPI pins
  LAN_SPI_CONFIG_SS_PIN_AS_OUTPUT();

  // configure SPI
  LAN_SPI_SET_SS_OFF();

  // Присваиваем начальные значения для приемных буффенров
  SPI_RxBuffer=SPI_RxBuffer_0;
  SPI_Rx_Cur_Buffer=0;
  SPI_RxRdyBuffer=SPI_RxBuffer_1;
  UART_Tx_Buffer=SPI_RxBuffer_1;

  // Заполняем буфера (можно не делать :)

#ifdef _MASTER_
  // Заполняем буффер передачи
  for (i_=0; i_<10; i_++)
    SPI_TxBuffer[i_]='A'+i_;
#else
  // Заполняем буффер передачи
  for (i_=0; i_<10; i_++)
    SPI_TxBuffer[i_]='a'+i_;
#endif

  // Начальные уставки FIFO
  RX_GET_FIFO_Ptr=FIFO_DATA_START;
  TX_SET_FIFO_Ptr=FIFO_DATA_START;
  RX_CHK_FIFO_Ptr=FIFO_DATA_START;

  SPI_TxBuffer[FIFO_SET_INDEX]=FIFO_DATA_START;
  SPI_TxBuffer[FIFO_GET_INDEX]=FIFO_DATA_START;

  SPI_RxBuffer_0[FIFO_SET_INDEX]=FIFO_DATA_START;
  SPI_RxBuffer_0[FIFO_GET_INDEX]=FIFO_DATA_START;
  SPI_RxBuffer_1[FIFO_SET_INDEX]=FIFO_DATA_START;
  SPI_RxBuffer_1[FIFO_GET_INDEX]=FIFO_DATA_START;

  // ------> Радио - сброс
  Radio_State=Radio_Reset;

  Timer_Cnt=0;
  WD_Cnt=0;
  WD_Rx_Time=0;
  Radio_Ready=0;

  // Запускаем таймер и PKT - поехали!
  ENABLE_T2_INTERRUPT();
  RESET_PKT_INTERRUPT_FLAG;
  ENABLE_PKT_INTERRUPT;
}

//------------------------------------------------------------------------------------------------
// 											Вспомогательные
//------------------------------------------------------------------------------------------------

// Последовательная запись в регистры TR24
void TR24_Send_Regs (const u08 * seq_, char len_)
{
  while (len_) {
     if (len_ & 3) {
        SPDR = *seq_;
        while(!(SPSR & (1<<SPIF))); // Wait for transmission complete
    } else {
      LAN_SPI_SET_SS_OFF();
        if (*seq_)
          RESET_PKT_INTERRUPT_FLAG;
      LAN_SPI_SET_SS_ON();
    }
    seq_++;
    len_--;
  }
}

// Запись в регистр TR24
void LanSPIRegAccess(u08 reg, u16 data)
{
  LAN_SPI_SET_SS_ON();
  SPDR = reg;
  while(!(SPSR & (1<<SPIF)));
  SPDR = (unsigned char) (data >> 8);
  while(!(SPSR & (1<<SPIF)));
  SPDR = ( data & 0xff );
  while(!(SPSR & (1<<SPIF)));
  LAN_SPI_SET_SS_OFF();
}

// Отсылка данных в FIFO TR24
void TR24_Data_Put(void)
{
  u08 len_;
  u08 * TX_;
  u08 CS_;
  u08 tmp_;

  LAN_SPI_SET_SS_ON();

  SPDR = RF_TX_FIFO_REG;
  while(!(SPSR & (1<<SPIF))); // Wait for transmission complete

  // длина передаваемых данных
  len_=SPI_BUFFER_SIZE;
  SPDR = len_;
  len_--;
  CS_=CS_INITIAL;
  TX_=SPI_TxBuffer;
  while(!(SPSR & (1<<SPIF))); // Wait for transmission complete

  while (len_)
  {
    tmp_=*TX_;
    SPDR = tmp_;
    // CRC 8
    CS_ = crc8_table[CS_ ^ tmp_];
    TX_++;
    len_--;
    while(!(SPSR & (1<<SPIF))); // Wait for transmission complete
  }
  SPDR = CS_;
  *TX_=CS_;
  while(!(SPSR & (1<<SPIF))); // Wait for transmission complete

  LAN_SPI_SET_SS_OFF();
}

// Прием данных из FIFO TR24 с проверкой
void TR24_Data_Get(void)
{
  u08 len_;
  u08 tmp_;
  u08 RX_len_;
  u08 CS_;
  u08 * RX_;

  LAN_SPI_SET_SS_ON();

  SPDR = RF_TX_FIFO_REG + 0x80;
  while(!(SPSR & (1<<SPIF))); // Wait for transmission complete

  // длина принимаемых данных
  len_=SPI_BUFFER_SIZE;
  SPDR = len_;
  len_--;
  RX_=SPI_RxBuffer;
  CS_=CS_INITIAL;
  while(!(SPSR & (1<<SPIF))); // Wait for transmission complete
  RX_len_=SPDR;

  while (len_)
  {
    SPDR = 0; // Не имеет значения
    while(!(SPSR & (1<<SPIF))); // Wait for transmission complete
    tmp_=SPDR;
    *RX_=tmp_;
    // CRC 8
    CS_ = crc8_table[CS_ ^ tmp_];
    RX_++;
    len_--;
  }

  SPDR = 0; // Не имеет значения
  while(!(SPSR & (1<<SPIF))); // Wait for transmission complete
  *RX_=SPDR;

  if ( (RX_len_==SPI_BUFFER_SIZE)
      && (CS_==SPI_RxBuffer[FIFO_CS_INDEX])
     )
  {
    // Всё ok, переключаем буфферы
    if (SPI_Rx_Cur_Buffer)
    {
      SPI_Rx_Cur_Buffer=0;
      SPI_RxBuffer=SPI_RxBuffer_0;
      SPI_RxRdyBuffer=SPI_RxBuffer_1;
    }
    else
    {
      SPI_Rx_Cur_Buffer=1;
      SPI_RxBuffer=SPI_RxBuffer_1;
      SPI_RxRdyBuffer=SPI_RxBuffer_0;
    }
    WD_Cnt=0; // Сброс WD
    Radio_Active=1; // Обмен работает
  }
  else
  {
    // Проблемы
    WD_Cnt++; // Увеличиваем счетчик WD
  }
  LAN_SPI_SET_SS_OFF();
}

//------------------------------------------------------------------------------------------------
// 									Timer interrupt ( LanPktIsr )
//------------------------------------------------------------------------------------------------
// Вызывается каждые 1 мс
#pragma vector = TIMER2_COMP_vect
__interrupt void Timer_1ms_interrupt_handler(void)
{	
#ifndef _MASTER_
  u08 tmp_;
  u08 idx_;
  u08 * msg_;
#endif

  // Увеличиваем счетчик милисекундных прерываний
  Timer_Cnt ++;

  // Машина состояний радио
  switch (Radio_State)
  {
    case Radio_Reset:
      LAN_DRIVE_RESETN_PIN_LOW();
      Radio_Ready=0;
      Timer_Cnt = 0;
      Radio_State=Radio_Reset1; // Next state >>
      break;
//------------------------------------------------------------------------------
    case Radio_Reset1:
      if ( Timer_Cnt == 3 )
      {
        Timer_Cnt = 0;
        LAN_DRIVE_RESETN_PIN_HIGH();
        Radio_State=Radio_Reset2; // Next state >>
      }
      break;
//------------------------------------------------------------------------------
    case Radio_Reset2:
      if ( Timer_Cnt == 4 )
      {
        // запись регистров 48...58
        Timer_Cnt = 0;
        TR24_Send_Regs(TR24_Reg_Grp1,TR24_Reg_Grp1_Len);
        Radio_State=Radio_Reset3; // Next state >>
      }
      break;
//------------------------------------------------------------------------------
    case Radio_Reset3:
      if ( Timer_Cnt == 2 )
      {
        // запись регистров 0...28
        Timer_Cnt = 0;
        TR24_Send_Regs(TR24_Reg_Grp2,TR24_Reg_Grp2_Len);
        Radio_State=Radio_Reset4; // Next state >>
      }
      break;
//------------------------------------------------------------------------------
    case Radio_Reset4:
  // --->>> Запускаем радио
      Radio_Ready=1;

#ifdef _MASTER_

    // ---- MASTER MODE

      // ------> Ждем следующего тика таймера для отправки
      Radio_State=TX_Timer;

#else
    // ---- SLAVE MODE

      // ------> Переходим в режим приема
      // IDLE mode
      LanSPIRegAccess( RF_TX_RX_CONTROL, (LanRFPLLChNb + RF_IDLE_MODE) );
      // Enable the Receive Sequence
      LanSPIRegAccess( RF_TX_RX_CONTROL, (LanRFPLLChNb + RF_RECV_MODE) );
      // сбрасываем ложное прерывание PKT
      RESET_PKT_INTERRUPT_FLAG;
      // clear RX FIFO point
      LanSPIRegAccess( RF_FIFO_RD_PTR, (RF_CLEAR_TXFIFO | RF_CLEAR_RXFIFO) );
//    LanTimerPktMS = 5; // Таймаут ожидания пакета

      // ------> Ждем прихода данных
      Radio_State=RX_Pkt;
    // ----
#endif
      break;
//------------------------------------------------------------------------------
    case RX_Pkt:
      WD_Cnt++; // Увеличиваем счетчик ошибок
    case TX_Timer:

#ifdef _MASTER_
      if (Timer_Cnt&1) // каждый второй тик
      {

        // ------> Передаем
        // IDLE mode
        LanSPIRegAccess( RF_TX_RX_CONTROL, (LanRFPLLChNb + RF_IDLE_MODE) );
        // Enable the Transmit Sequence
        LanSPIRegAccess( RF_TX_RX_CONTROL, (LanRFPLLChNb + RF_TRAN_MODE) );
        // сбрасываем ложное прерывание PKT
        RESET_PKT_INTERRUPT_FLAG;
        // clear TX FIFO point
        LanSPIRegAccess( RF_FIFO_RD_PTR, (RF_CLEAR_TXFIFO | RF_CLEAR_RXFIFO) );
        // write TX FIFO
        TR24_Data_Put();

        // ------> Ждем окончания передачи
        Radio_State=TX_Pkt;
      }
  // ----
#endif
      break;

    case TX_Pkt:
      WD_Cnt++; // Увеличиваем счетчик ошибок
      break;

  } // end switch

//------------------------------------------------------------------------------

  // TX очередь

  // ------> Проверяем очередь
  // Если в очереди есть данные на отправку
  if (UART_EMPTY())
  {
    UART_Tx_Buffer=SPI_RxRdyBuffer;
#ifndef _MASTER_
    // Режим Echo
    if (SW_Pos==0)
      UART_Tx_Buffer=SPI_TxBuffer; // Берем из входного буффера
#endif

    if (UART_Tx_Buffer[FIFO_SET_INDEX]!=RX_GET_FIFO_Ptr)
    {
      // Берем следующий индекс
      RX_GET_FIFO_Ptr++;
      if (RX_GET_FIFO_Ptr >= SPI_BUFFER_SIZE-1)
        RX_GET_FIFO_Ptr=FIFO_DATA_START; // начальный индекс данных
       // Сохраняем новый индекс
      SPI_TxBuffer[FIFO_GET_INDEX]=RX_GET_FIFO_Ptr;
      // Передаем байт в COM-порт
      UDR=UART_Tx_Buffer[RX_GET_FIFO_Ptr];
    }
  }
//------------------------------------------------------------------------------

// Обработка NMEA

#ifndef _MASTER_
    if (SPI_RxRdyBuffer[FIFO_SET_INDEX]!=RX_CHK_FIFO_Ptr)
    {
      // Берем следующий индекс
      RX_CHK_FIFO_Ptr++;
      if (RX_CHK_FIFO_Ptr >= SPI_BUFFER_SIZE-1)
        RX_CHK_FIFO_Ptr=FIFO_DATA_START; // начальный индекс данных
      // анализируем байт
      SW_Check(SPI_RxRdyBuffer[RX_CHK_FIFO_Ptr]);
    }
#endif

//------------------------------------------------------------------------------

#ifndef _MASTER_

  // Посылка ACK и переключение режима МУ/ДУ
  if (SW_ACK)
  {
    if ((Timer_Cnt>=SW_Activation_Timeout)||(PCK_End))
    {
      if ((SW_ACK==SW_ACK_OK) & (SW_Pos!=SW_Cur))
      {
        SW_Pos=SW_Cur; // Переопределяем переключатель

        // сбрасываем указатели FIFO для локального UART
        if (SW_Pos)
          RX_GET_FIFO_Ptr=UART_Tx_Buffer[FIFO_SET_INDEX];
        else
          RX_GET_FIFO_Ptr=SPI_TxBuffer[FIFO_SET_INDEX];
      }

      // Отсылаем сооющение
      msg_=(u08 *)(SW_ACK==SW_ACK_OK ? SW_ACK_Seq : SW_NACK_Seq);
      tmp_=SPI_TxBuffer[FIFO_SET_INDEX];

      // Копируем сообщение
      for (idx_=0; idx_<SW_ACK_Len; idx_++)
      {
        // Берем следующий индекс
        tmp_++;
        if (tmp_ >= SPI_BUFFER_SIZE-1)
          tmp_ = FIFO_DATA_START;
        // Берем байт
        SPI_TxBuffer[tmp_]=*msg_;
        msg_++;
      }
      SPI_TxBuffer[FIFO_SET_INDEX]=tmp_;

      // Сбрасываем ACK
      SW_ACK=0;

    }
  }

#endif

//------------------------------------------------------------------------------
//          ДИАГНОСТИКА!
  // Посылка предупреждения об отсутствии приема
    if ((Radio_Ready) && (WD_Cnt>WD_Pre_Treshold))
        SPI_TxBuffer[FIFO_GET_INDEX]=0; // признак отсутствия приема
    else
        SPI_TxBuffer[FIFO_GET_INDEX]=RX_GET_FIFO_Ptr; // нормальный режим

  // Прием предупреждения
    if (SPI_RxRdyBuffer[FIFO_GET_INDEX]==0)
      WD_Rx_Time++;
    else
      WD_Rx_Time=0;


  // Перезапуск радио
    if ((Radio_Ready) && ((WD_Cnt>WD_Treshold)||(WD_Rx_Time>WD_Rx_Treshold)))
    {
      WD_Cnt=0; // Сбрасываем счетчик ошибок
      WD_Rx_Time=0;
      SPI_RxRdyBuffer[FIFO_GET_INDEX]=FIFO_DATA_START; // принудительная инициализация
      Radio_Ready=0;
      Radio_State=Radio_Reset; // RESET
      Radio_Active=0;
    }
}



//------------------------------------------------------------------------------------------------
// 									PKT interrupt ( LanPktIsr )
//------------------------------------------------------------------------------------------------
#pragma vector = INT2_vect
__interrupt void PKT_INT (void)
{

#ifdef _MASTER_

  // ---- MASTER MODE
  switch (Radio_State)
  {
    // Ждали PKT при передаче
  case TX_Pkt:

    // ------> Переходим в режим приема
    // IDLE mode
    LanSPIRegAccess( RF_TX_RX_CONTROL, (LanRFPLLChNb + RF_IDLE_MODE) );
    // Enable the Receive Sequence
    LanSPIRegAccess( RF_TX_RX_CONTROL, (LanRFPLLChNb + RF_RECV_MODE) );
    // clear RX FIFO point
    LanSPIRegAccess( RF_FIFO_RD_PTR, (RF_CLEAR_TXFIFO | RF_CLEAR_RXFIFO) );
    // ------> Ждем прихода данных
    Radio_State=RX_Pkt;
    break;

    // Ждали PKT при приеме
  case RX_Pkt:


    // ------> Забираем данные
//    TR24_DataExchange( RF_TX_FIFO_REG + 0x80, (u08*) SPI_TxBuffer, (u08*) SPI_RxBuffer, SPI_BUFFER_SIZE);
    TR24_Data_Get();

    // ------> Ждем следующего тика таймера для отправки
    Radio_State=TX_Timer;
    break;
  }

#else

  // ---- SLAVE MODE
  switch (Radio_State)
  {
    // Ждали PKT при передаче
  case TX_Pkt:
    // ------> Переходим в режим приема
    // IDLE mode
    LanSPIRegAccess( RF_TX_RX_CONTROL, (LanRFPLLChNb + RF_IDLE_MODE) );
    // Enable the Receive Sequence
    LanSPIRegAccess( RF_TX_RX_CONTROL, (LanRFPLLChNb + RF_RECV_MODE) );
    // clear RX FIFO point
    LanSPIRegAccess( RF_FIFO_RD_PTR, (RF_CLEAR_TXFIFO | RF_CLEAR_RXFIFO) );
//    LanTimerPktMS = 5; // Таймаут ожидания пакета

    // ------> Ждем прихода данных
    Radio_State=RX_Pkt;
    break;

    // Ждали PKT при приеме
  case RX_Pkt:

// ------> Забираем данные
//    TR24_DataExchange( RF_TX_FIFO_REG + 0x80, (u08*) SPI_TxBuffer, (u08*) SPI_RxBuffer, SPI_BUFFER_SIZE);
    TR24_Data_Get();

    // ------> Передаем
    // IDLE mode
    LanSPIRegAccess( RF_TX_RX_CONTROL, (LanRFPLLChNb + RF_IDLE_MODE) );
    // Enable the Transmit Sequence
    LanSPIRegAccess( RF_TX_RX_CONTROL, (LanRFPLLChNb + RF_TRAN_MODE) );
    // clear TX FIFO point
    LanSPIRegAccess( RF_FIFO_RD_PTR, (RF_CLEAR_TXFIFO | RF_CLEAR_RXFIFO) );
    // write TX FIFO
//    TR24_DataExchange( RF_TX_FIFO_REG, (u08*) SPI_TxBuffer, 0, SPI_BUFFER_SIZE);
    TR24_Data_Put();
    // ------> Ждем окончания передачи
    Radio_State=TX_Pkt;

    break;
  }
  // ----
#endif
}
