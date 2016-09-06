/**
 * @brief 5位数码管动态显示
 * @file
 * @author shenxf 380406785@@qq.com
 * @version v1.0.0
 * @date 2016-09-03
 * 
 * 5位数码管动态显示
 * 函数列表
 * @sa disp_init 初始化
 * @sa disp_on  开显示
 * @sa disp_off 关显示
 * @sa disp_play 将数传递至显示缓冲区
 * @sa disp_fill 填充字段码
 * @sa disp_filln 指定数位填充
 */ 
#include <avr/interrupt.h>
#include "disp.h"

/**
 * @brief 存储在FLASH的七段数码管显示编码，依次0、1、2、3、4、5、6、7、8、9
 */ 
__flash const uint8_t digitcode[10] = 
{0x3fU,0x06,0x5bU,0x4fU,0x66U,0x6dU,0x7dU,0x07,0x7fU,0x6fU};

/**
 * @brief 七段数码管显示编码显示缓冲区，下标为0时表示个位，依次类推
 */ 
uint8_t disp_buf[5];
volatile uint8_t disp_index;/**<当前显示的数位号0-4*/

/**
 * @brief 定时器2比较匹配中断服务程序
 * 
 * 2ms一次中断服务，关闭当前位显示，更新下一个数位并显示，5位数码管显示刷新率10ms
 */ 
ISR(TIMER2_COMPA_vect)
{
  uint8_t ind;
  uint8_t scode;
  ind = disp_index;
  
  /*关闭当前数位显示，位选置1关闭*/
  if(0 == ind)
  {
    DIGIT0_PORT |= _BV(DIGIT_PIN0);
  }
  else if(1U == ind)
  {
    DIGIT1_PORT |= _BV(DIGIT_PIN1);
  }
  else if(2U == ind)
  {
    DIGIT2_PORT |= _BV(DIGIT_PIN2);
  }
  else if(3U == ind)
  {
    DIGIT3_PORT |= _BV(DIGIT_PIN3);
  }
  else if(4U == ind)
  {
    DIGIT4_PORT |= _BV(DIGIT_PIN4);
  }
  else
  {
    ;/* no deal with */
  }
  
  /*更新下一个数位显示编码，段置1数码管段亮*/
  ind++;
  if(ind > 4U)
  {
    ind = 0;
  }
  scode = disp_buf[ind];
  SEGC_POTR = (scode & 0x3fU);
  if(0x40U == (scode & 0x40U))
  {
    SEGD_PORT |= _BV(SEGD_PIN6);
  }
  else
  {
    SEGD_PORT &= ~_BV(SEGD_PIN6);
  }
  if(0x80U == (scode & 0x80U))
  {
    SEGD_PORT |= _BV(SEGD_PIN7);
  }
  else
  {
    SEGD_PORT &= ~_BV(SEGD_PIN7);
  }
  
  /*显示下一个数码，位选置0显示*/
  if(0 == ind)
  {
    DIGIT0_PORT &= ~_BV(DIGIT_PIN0);
  }
  else if(1U == ind)
  {
    DIGIT1_PORT &= ~_BV(DIGIT_PIN1);
  }
  else if(2U == ind)
  {
    DIGIT2_PORT &= ~_BV(DIGIT_PIN2);
  }
  else if(3U == ind)
  {
    DIGIT3_PORT &= ~_BV(DIGIT_PIN3);
  }
  else if(4U == ind)
  {
    DIGIT4_PORT &= ~_BV(DIGIT_PIN4);
  }
  else
  {
    ;/* no deal with */
  }
  
  /*保存下一个数位号*/
  disp_index = ind;  
  
}

/**
 */ 
void disp_init(void)
{
  uint8_t i;
  
  /**/
  SEGC_POTR = 0x00;
  SEGC_DDR  = 0x00;
  SEGD_PORT &= ~(_BV(SEGD_PIN6)|_BV(SEGD_PIN7));
  SEGD_DDR  &= ~(_BV(SEGD_PIN6)|_BV(SEGD_PIN7));

  /**/
  DIGIT4_PORT &= ~_BV(DIGIT_PIN4);
  DIGIT3_PORT &= ~_BV(DIGIT_PIN3);
  DIGIT2_PORT &= ~_BV(DIGIT_PIN2);
  DIGIT1_PORT &= ~_BV(DIGIT_PIN1);
  DIGIT0_PORT &= ~_BV(DIGIT_PIN0);
  DIGIT4_DDR  &= ~_BV(DIGIT_PIN4);
  DIGIT3_DDR  &= ~_BV(DIGIT_PIN3);
  DIGIT2_DDR  &= ~_BV(DIGIT_PIN2);
  DIGIT1_DDR  &= ~_BV(DIGIT_PIN1);
  DIGIT0_DDR  &= ~_BV(DIGIT_PIN0);
  
  /**/
  for(i = 0;i<5;i++)
  {
    disp_buf[i] = 0;
  }
  disp_index = 0;
  
  /**/
  TCCR2A = _BV(WGM21);
  OCR2A = 249;
  TCNT2 = 0;
  TCCR2B = 0;
}

/**
 */ 
void disp_on(void)
{
  /**/
  SEGC_DDR  = 0x3fU;
  SEGD_DDR  = _BV(SEGD_PIN6)|_BV(SEGD_PIN7);
  DIGIT4_DDR  |= _BV(DIGIT_PIN4);
  DIGIT3_DDR  |= _BV(DIGIT_PIN3);
  DIGIT2_DDR  |= _BV(DIGIT_PIN2);
  DIGIT1_DDR  |= _BV(DIGIT_PIN1);
  DIGIT0_DDR  |= _BV(DIGIT_PIN0);
  
  /**/
  TIMSK2 = _BV(OCIE2A);
  TCCR2B |= _BV(CS22)|_BV(CS21);      
}

/**
 */ 
void disp_off(void)
{
  /**/
  SEGC_DDR  = 0x00;
  SEGD_DDR  &= ~(_BV(SEGD_PIN6)|_BV(SEGD_PIN7));
  DIGIT4_DDR  &= ~_BV(DIGIT_PIN4);
  DIGIT3_DDR  &= ~_BV(DIGIT_PIN3);
  DIGIT2_DDR  &= ~_BV(DIGIT_PIN2);
  DIGIT1_DDR  &= ~_BV(DIGIT_PIN1);
  DIGIT0_DDR  &= ~_BV(DIGIT_PIN0);
  
  /**/
  TCCR2B &= ~(_BV(CS22)|_BV(CS21));
  TIFR2 |= _BV(OCF2A);
  TIMSK2 &= ~_BV(OCIE2A);
}

/**
 */ 
void disp_play(uint32_t num)
{
  uint32_t nm;
  uint8_t dgt,ind;
  uint8_t digits[5];
  nm = num;
  if(nm<100000UL)
  {
    for(ind = 0;ind < 5;ind++)
    {
      digits[ind] = 0;
    }
    ind = 0;
    do
    {
      dgt = nm % 10;
      digits[ind] = dgt;
      nm /= 10;
      ind++;
    }
    while(0 != nm);
    dgt = nm % 10;
    digits[ind] = dgt;
    disp_buf[4] = digitcode[digits[4]] | DPOINT;
    for(ind = 0;ind < 4;ind++)
    {
      disp_buf[ind] = digitcode[digits[ind]];
    }
  }
}

/**
 */ 
void disp_fill(uint8_t segs)
{
  uint8_t i;
  for(i = 0;i < 5;i++)
  {
    disp_buf[i] = segs;
  }    
}

/**
 */ 
void disp_filln(uint8_t segs,uint8_t digit)
{
  disp_buf[digit] = segs;
}

