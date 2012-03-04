#ifndef __NMEA_H__
#define __NMEA_H__

// Тип подтверждения
#define SW_ACK_NU 0
#define SW_ACK_ERR 1
#define SW_ACK_OK 2

// Время ожидания после незавершенного пакета
#define SW_Activation_Timeout 100

// Текст подтверждения
#define SW_ACK_Len  12
extern const u08 SW_ACK_Seq [SW_ACK_Len];
extern const u08 SW_NACK_Seq [SW_ACK_Len];

extern u08 SW_Pos;      // Текущий режим
extern u08 SW_Cur;      // Режим после последнего изменения
extern u08 SW_ACK;      // Состояние подтверждения
extern u08 PCK_End;     // Флаг окончания посылки NMEA

// Проверка переключателя (инициализация)
void SW_Init(void);
// Проверка переключателя (автомат)
void SW_Check(u08 chr_);

#endif
