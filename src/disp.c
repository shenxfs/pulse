/**
 * @brief 5位数码管动态显示
 * @file disp.c
 * @author shenxf 380406785@@qq.com
 * @version V1.2.0
 * @date 2016-10-24
 * 
 * 5位公阴数码管动态显示，
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
 *@var __flash const uint8_t digitcode[10]
 * @brief 存储在FLASH的七段数码管显示编码，依次0、1、2、3、4、5、6、7、8、9
 */ 
__flash const uint8_t digitcode[10] = 
{0x3fU,0x06,0x5bU,0x4fU,0x66U,0x6dU,0x7dU,0x07,0x7fU,0x6fU};

/**
 * @var disp_buf[5]
 * @brief 七段数码管显示编码显示缓冲区，下标为0时表示个位，依次类推
 */ 
uint8_t disp_buf[5];

volatile uint8_t disp_index;/**<当前显示的数位号0-4*/

/**
 * 
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
  SEGC_PORT = (scode & 0x3fU);
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
*@fn void disp_init(void)
*@brief 初始化
*
*动态数码管显示初始化，设置段端口、位选端口为高阻输入，定时器2设置为CTC模式
 */ 
void disp_init(void)
{
  uint8_t i;
  
  /* 段端口初始化，PC0～PC5对应a~f段，PD3～PD4对应g、dp段，高阻输入
  ＊ PC0(A0)-->a
  *  PC1(A1)-->b
  *	 PC2(A2)-->c
  *  PC3(A3)-->d
  *  PC4(A4)-->e
  *  PC5(A5)-->f
  *  PD3(D3)-->g
  *  PD4(D4)-->dp
  */
  SEGC_PORT = 0x00;
  SEGC_DDR  = 0x00;
  SEGD_PORT &= ~(_BV(SEGD_PIN6)|_BV(SEGD_PIN7));
  SEGD_DDR  &= ~(_BV(SEGD_PIN6)|_BV(SEGD_PIN7));

  /*位选端口初始化，DS0～DS4对应个位～万位，高阻输入
  *DS4-->PD7(D7)
  *DS3-->PB0(D8)
  *DS2-->PB2(D10)
  *DS1-->PB3(D11)
  *DS0-->PB4(D12)
  */
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
  
  /*七段数码管编码缓冲区，下标0对应DS0*/
  for(i = 0;i<5U;i++)
  {
    disp_buf[i] = 0;
  }
  disp_index = 0;
  
  /*定时器2初始化，CTC模式，计数器清零，比较匹配寄存器赋值249，T2分频数250，关闭T2时钟*/
  TCCR2A = _BV(WGM21);
  OCR2A = 249U;
  TCNT2 = 0;
  TCCR2B = 0;
}

/**
 *@fn void disp_on(void)
 *@brief 打开数码管显示
 * @sa disp_off 关显示
 */
void disp_on(void)
{
  /*设置段端口为输出*/
  SEGC_DDR  = 0x3fU;
  SEGD_DDR  |= _BV(SEGD_PIN6)|_BV(SEGD_PIN7);

  /*设置位选端口为输出*/
  DIGIT4_DDR  |= _BV(DIGIT_PIN4);
  DIGIT3_DDR  |= _BV(DIGIT_PIN3);
  DIGIT2_DDR  |= _BV(DIGIT_PIN2);
  DIGIT1_DDR  |= _BV(DIGIT_PIN1);
  DIGIT0_DDR  |= _BV(DIGIT_PIN0);
  
  /*允许T2比较匹配中断，开启T2时钟，预分频数128*/
  TIMSK2 = _BV(OCIE2A);
  TCCR2B |= _BV(CS22)|_BV(CS21);      
}

/**
 *@brief 关闭数码管显示
 * @sa disp_on  开显示
*/
void disp_off(void)
{
  /*设置段端口为高阻输入*/
  SEGC_DDR  = 0x00;
  SEGD_DDR  &= ~(_BV(SEGD_PIN6)|_BV(SEGD_PIN7));

  /*设置位选端口为高阻输入*/
  DIGIT4_DDR  &= ~_BV(DIGIT_PIN4);
  DIGIT3_DDR  &= ~_BV(DIGIT_PIN3);
  DIGIT2_DDR  &= ~_BV(DIGIT_PIN2);
  DIGIT1_DDR  &= ~_BV(DIGIT_PIN1);
  DIGIT0_DDR  &= ~_BV(DIGIT_PIN0);
  
  /*关闭T2时钟，禁止T2比较匹配中断，请T2比较匹配中断标志*/
  TCCR2B &= ~(_BV(CS22)|_BV(CS21));
  TIFR2 |= _BV(OCF2A);
  TIMSK2 &= ~_BV(OCIE2A);
}

/**
 *@brief 将整型数转换成七段数码管编码，并存入缓冲区
 *@param[in] num 长整型数，单位是0.1ms
 *
 *只接收小于1000000的数，只能显示5位十进制数
 * @sa disp_on  开显示
 * @sa disp_off 关显示
 * @sa disp_fill 填充字段码
 * @sa disp_filln 指定数位填充
 * @sa digitcode[]
 */ 
void disp_play(uint32_t num)
{
  uint32_t nm;
  uint8_t dgt,ind;
  uint8_t digits[5];
  nm = num;
  if(nm<100000UL)
  {
    /*五位十进制数字，初始值0*/
    for(ind = 0;ind < 5U;ind++)
    {
      digits[ind] = 0;
    }

    /*将整型数转换十进制数*/
    ind = 0;
    do
    {
      dgt = (uint8_t)(nm % 10U);
      digits[ind] = dgt;
      nm /= 10;
      ind++;
    }
    while(0 != nm);
    dgt = (uint8_t)(nm % 10);
    digits[ind] = dgt;

    /*将十进制数转换成七段数码管编码*/
    disp_buf[4] = digitcode[digits[4]] | DPOINT;/*最高位总是带小数点的，因此加上dp位*/
    for(ind = 0;ind < 4U;ind++)
    {
      disp_buf[ind] = digitcode[digits[ind]];
    }
  }
}

/**
 *@brief 将七段数码管编码填充全部缓冲区
 *@param[in] segs 七段数码管编码
 * @sa disp_on  开显示
 * @sa disp_off 关显示
 * @sa disp_play 将数传递至显示缓冲区
 * @sa disp_filln 指定数位填充
 */ 
void disp_fill(uint8_t segs)
{
  uint8_t i;
  for(i = 0;i < 5U;i++)
  {
    disp_buf[i] = segs;
  }    
}

/**
 *@brief 按指定数位将七段数码管编码填充至缓冲区
 *@param[in] segs 七段数码管编码
 *@param[in] digit 指定数位
 * @sa disp_on  开显示
 * @sa disp_off 关显示
 * @sa disp_play 将数传递至显示缓冲区
 * @sa disp_fill 填充字段码
 */ 
void disp_filln(uint8_t segs,uint8_t digit)
{
  disp_buf[digit] = segs;
}

