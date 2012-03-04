#include "main.h"

u08 SW_Pos;
u08 SW_Cur;
u08 SW_Pos_Pre;
u08 SW_Seq_Idx;
u08 SW_ACK;
u08 SW_ACK_Pre;
u08 PCK_End;


// $PRCON,a*hh<CR><LF>
//#define SW_Seq_Len  13
//const u08 SW_Com_Seq [SW_Seq_Len]={'$','P','R','C','O','N',',','1','*','A','B',0x0D,0x0A};
//const u08 SW_Off_Seq [SW_Seq_Len]={'$','P','R','C','O','N',',','0','*','A','B',0x0D,0x0A};

// $PAСK,a*hh<CR><LF>
const u08 SW_ACK_Seq [SW_ACK_Len]={'$','P','A','C','K',',','1','*','0','4',0x0D,0x0A};
const u08 SW_NACK_Seq [SW_ACK_Len]={'$','P','A','C','K',',','0','*','0','5',0x0D,0x0A};

// Инициализация переключателя режимов
void SW_Init(void)
{
    SW_Pos=0; // Состояние RC по умолчанию
    SW_Pos_Pre=0; // Состояние RC предварительное
    SW_Seq_Idx=0;
    SW_ACK_Pre=0;
    PCK_End=0;
}

// Проверка переключателя (автомат)
void SW_Check(u08 chr_)
{
  // Условие запуска
  if (chr_=='$')
  {
     SW_Seq_Idx=0;
     SW_ACK_Pre=0;
     return;
  }

  SW_Seq_Idx++;

  switch (SW_Seq_Idx)
  {
  // $P
  case 1:
    if (chr_!='P')
      SW_Seq_Idx=0;
    break;

  // $PR
  case 2:
    if (chr_!='R')
      SW_Seq_Idx=0;
    break;

  // $PRC
  case 3:
    if (chr_!='C')
      SW_Seq_Idx=0;
    break;

  // $PRCO
  case 4:
    if (chr_!='O')
      SW_Seq_Idx=0;
    break;

  // $PRCON
  case 5:
    if (chr_!='N')
      SW_Seq_Idx=0;
    break;

  // $PRCON,
  case 6:
    if (chr_!=',')
      SW_Seq_Idx=0;
    break;

  // $PRCON,x -> SW_Pos_Pre=x
  case 7:
    if (chr_=='0')
      SW_Pos_Pre=0;
    else
    {
      if (chr_=='1')
        SW_Pos_Pre=1;
      else
        SW_Seq_Idx=0;
    }

    break;

  // $PRCON,x*
  case 8:
    if (chr_!='*')
      SW_Seq_Idx=0;
    break;

  // Контрольная сумма (Hi)
  case 9:
    SW_ACK_Pre=SW_ACK_OK;
    if (chr_!='5')
      SW_ACK_Pre=SW_ACK_ERR;
    break;

  // Контрольная сумма (Lo)
  case 10:
    if (SW_Pos_Pre)
    {
      if (chr_!='D')
        SW_ACK_Pre=SW_ACK_ERR;
    }
    else
    {
      if (chr_!='C')
        SW_ACK_Pre=SW_ACK_ERR;
    }
    break;

  // $PRCON,a*hh<CR>
  case 11:
    if (chr_!=0x0D)
      SW_Seq_Idx=0;
    break;

  // $PRCON,a*hh<CR><LF>
  case 12:
    if (chr_==0x0A)
    {
      SW_ACK=SW_ACK_Pre;
      if (SW_ACK_Pre==SW_ACK_OK)
        // Переключаем
        SW_Cur=SW_Pos_Pre;

      // Взводим таймер
      Timer_Cnt=0;
    }
    SW_Seq_Idx=0;
    break;

  // Перестраховка
  default:
    SW_Seq_Idx=0;
    SW_ACK_Pre=0;
    break;
  }
}

