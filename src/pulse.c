/**
 * @brief 单脉冲源
 * @file
 * @author shenxf 380406785@@qq.com
 * @version V1.0.0
 * @date 2016-09-03
 * 
 * 函数列表
 * @ref pls_init
 * @ref pls_set_pulse
 * @ref pls_set_mode
 * @ref pls_set_param
 * @ref pls_get_sta
 * @ref pls_start
 * @ref pls_get_delay
 * @ref pls_get_width
 */
#include <avr/interrupt.h>
#include "pulse.h"

/**
 * @brief   延迟脉宽结构类型
 * @struct  spls_t
 */
typedef struct pls_data
{
  uint16_t dlys;/**<延迟数，单位0.1ms*/
  uint16_t wtd;/**<脉宽数，单位0.1ms*/
}spls_t;

volatile uint8_t pls_mode;/**<模式，0自动，非0手动*/
volatile uint8_t pls_sta;/**<脉冲波状态*/
volatile uint8_t pls_index;/**<延迟脉宽结构类型数组下标*/
volatile uint16_t delays;/**<预产生延迟时间数*/
volatile uint16_t widths;/**<预产生脉冲时间数*/
volatile uint8_t pls_pre;/**<时基分频数减1,时基频率2MHz*/

/**
 * 自动模式延迟脉宽数据，共20组数据
 */
__flash const spls_t tims[20] = 
{
  { .dlys = 5000U,
    .wtd = 6000U,
  },
  { .dlys = 10000U,
    .wtd = 6000U,
  },
  { .dlys = 15000U,
    .wtd = 6000U,
  },
  { .dlys = 20000U,
    .wtd = 6000U,
  },
  { .dlys = 25000U,
    .wtd = 6000U,
  },
  { .dlys = 30000U,
    .wtd = 6000U,
  },
  { .dlys = 35000U,
    .wtd = 6000U,
  },
  { .dlys = 40000U,
    .wtd = 6000U,
  },
  { .dlys = 45000U,
    .wtd = 6000U,
  },
  { .dlys = 50000U,
    .wtd = 6000U,
  },
  { .dlys = 55000U,
    .wtd = 6000U,
  },
  { .dlys = 60000U,
    .wtd = 6000U,
  },
  { .dlys = 5000U,
    .wtd = 4000U,
  },
  { .dlys = 5000U,
    .wtd = 5000U,
  },
  { .dlys = 5000U,
    .wtd = 6000U,
  },
  { .dlys = 5000U,
    .wtd = 7000U,
  },
  { .dlys = 5000U,
    .wtd = 8000U,
  },
  { .dlys = 5000U,
    .wtd = 9000U,
  },
  { .dlys = 5000U,
    .wtd = 10000U,
  },
  { .dlys = 5000U,
    .wtd = 11000U,
  }
};

/**
 * @brief 外部中断0服务
 * 
 * 触发端口下降沿响应，启动定时器0,产生0.1ms或0.2ms时基
 */
ISR (INT0_vect)
{
    TCCR0B |= _BV(CS01);  
}

/**
 * @brief 定时器1比较匹配中断服务
 * 
 * 定时器1CTC模式，匹配时改变OC1A管脚的状态产生预期脉冲
 */
ISR (TIMER1_COMPA_vect)
{
  if(PULSE_STA_DELAY == pls_sta)
  {
    pls_sta = PULSE_STA_WIDTH;
    OCR1A = widths;
  }
  else if(PULSE_STA_WIDTH == pls_sta)
  {
    TCCR0B &= ~_BV(CS01);
    pls_sta = PULSE_STA_COMPLETE;
    TCNT1 = 0;
    TCNT0 = 0;
    TIMSK1 = 0;
    CLKOUT_PORT &= ~_BV(CLKOUT_PIN);
  }
  else
  {
    pls_sta = PULSE_STA_COMPLETE;
    TCCR0B &= ~_BV(CS01);
    TCNT1 = 0;
    TCNT0 = 0;    
    TIMSK1 = 0;
    CLKOUT_PORT &= ~_BV(CLKOUT_PIN);
  }
}

/**
 * 初始化
 */
void pls_init(void)
{
  /*触发端口及外部中断0初始化*/
  SPARK_DDR &= ~_BV(SPARK_PIN);
  SPARK_PORT |= _BV(SPARK_PIN);
  EICRA = _BV(ISC01);
  EIMSK = 0;

  /*脉冲输出端口初始化*/
  PULSE_DDR |= _BV(PULSE_PIN);
  PULSE_PORT &= ~_BV(PULSE_PIN);

  /*时基输入*/
  CLKIN_PORT &= _BV(CLKIN_PIN);
  CLKIN_DDR &= ~_BV(CLKIN_PIN);

  /*时基输出*/
  CLKOUT_PORT &= ~_BV(CLKOUT_PIN);
  CLKOUT_DDR |= _BV(CLKOUT_PIN);

  /*LED指示灯*/
  LED_DDR |= _BV(LED_PIN);
  LED_PORT &= ~_BV(LED_PIN);

  /*定时器0CTC模式，OC0输出0.1ms或0.2ms时基信号*/
  TCCR0A = _BV(COM0A0)|_BV(WGM01);
  TCCR0B = 0; 
  OCR0A = 99;
  TCNT0 = 0;

  /*定时器1CTC模式，OC1A管脚的状态产生预期脉冲*/
  TCCR1A = _BV(COM1A0);
  TCCR1B = _BV(WGM12)|_BV(CS12)|_BV(CS11);
  TIFR1 = _BV(OCF1A);
  OCR1A = 5000U;
  TCNT1 = 0;  
  
  /*全局变量初始化*/
  pls_sta = PULSE_STA_COMPLETE;
  pls_mode = 0;
  pls_index = 0;
  pls_pre = 99U;
  delays = 5000U;
  widths = 5000U;
}

/**
 * 
 */
void pls_set_pulse(uint32_t dly,uint16_t wtd)
{
  if(0 != pls_mode )
  {
    if(dly < 65536UL)
    {
      delays = dly;
      widths = wtd;
      if(delays < 5000U)
      {
        delays = 5000U;
      }
      if(widths < 4000U)
      {
        widths = 4000U;
      }
      pls_pre = 99U;
    }
    else if(dly < 100000UL)
    {
      delays = dly / 2;
      widths = wtd / 2;
      pls_pre = 199U;
    }
    else
    {
      delays = 49999UL;
      widths = wtd / 2;
      pls_pre = 199U;
    }
  }
}

/**
 */
void pls_set_mode(uint8_t mod)
{
  pls_mode = mod;
}

/**
 */
uint8_t pls_get_sta(void)
{
  return pls_sta;  
}

/**
 */
void pls_set_param(void)
{
  if(0 == pls_mode)
  {
    delays = tims[pls_index].dlys;
    widths = tims[pls_index].wtd;
    pls_pre = 99U;
    pls_index++;
    if(pls_index >= 20U)
    {
      pls_index = 0;
    }
  }
  OCR0A = pls_pre;
  OCR1A = delays;
  pls_sta = PULSE_STA_DELAY;
}

/**
 */
uint16_t pls_get_delay(void)
{
  uint16_t ret;
  ret = delays;
  if(pls_pre > 100U)
  {
    ret *= 2U;
  }
  return ret;
}

/**
 */
uint16_t pls_get_width(void)
{
  uint16_t ret;
  ret = widths;
  if(pls_pre > 100U)
  {
    ret *= 2U;
  }
  return ret;
}
