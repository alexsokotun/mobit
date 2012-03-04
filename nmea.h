#ifndef __NMEA_H__
#define __NMEA_H__

// ��� �������������
#define SW_ACK_NU 0
#define SW_ACK_ERR 1
#define SW_ACK_OK 2

// ����� �������� ����� �������������� ������
#define SW_Activation_Timeout 100

// ����� �������������
#define SW_ACK_Len  12
extern const u08 SW_ACK_Seq [SW_ACK_Len];
extern const u08 SW_NACK_Seq [SW_ACK_Len];

extern u08 SW_Pos;      // ������� �����
extern u08 SW_Cur;      // ����� ����� ���������� ���������
extern u08 SW_ACK;      // ��������� �������������
extern u08 PCK_End;     // ���� ��������� ������� NMEA

// �������� ������������� (�������������)
void SW_Init(void);
// �������� ������������� (�������)
void SW_Check(u08 chr_);

#endif
