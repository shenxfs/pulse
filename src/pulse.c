/**
 * @brief 单脉冲源
 * @file pulse.c 
 * @author shenxf 380406785@@qq.com
 * @version V1.1.1
 * @date 2016-10-18
 * 
 *单脉冲生产源模块，
 * 函数列表
 *@sa pls_init() 初始化
 *@sa pls_set_pulse() 手动设置时间参数
 *@sa pls_set_param() 自动设置时间参数
 *@sa pls_set_mode()  设置工作模式
 *@sa pls_get_mode() 取工作模式
 *@sa pls_set_sta()   设置脉冲状态
 *@sa pls_get_sta() 取脉冲状态
 *@sa pls_get_busy() 取工作状态
 *@sa pls_get_delay() 取延时数
 *@sa pls_get_width() 取脉宽数
 *@sa pls_strtou()    数字字符串转整型数
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

/**
 *脉冲波状态
 *@sa PULSE_STA_DELAY
 *@sa PULSE_STA_WIDTH
 *@sa PULSE_STA_COMPLETE
 */
volatile uint8_t pls_sta;

volatile uint8_t pls_index;/**<延迟脉宽结构类型数组下标*/
volatile uint16_t delays;/**<预产生延迟时间数*/
volatile uint16_t widths;/**<预产生脉冲时间数*/
volatile uint8_t pls_pre;/**<时基分频数减1,时基频率2MHz*/
volatile uint8_t pls_busy;/**<产生脉冲的工作标志，0未开始，不忙；1在进行，忙*/
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
    TIMSK1 |= _BV(OCIE1A);
    EIMSK &= ~_BV(INT0);
    LED_PORT &= ~_BV(LED_PIN);
    pls_busy = 1U;
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
    LED_PORT |= _BV(LED_PIN);
  }
  else if(PULSE_STA_WIDTH == pls_sta)
  {
    TCCR0B &= ~_BV(CS01);
    LED_PORT &= ~_BV(LED_PIN);
    pls_sta = PULSE_STA_COMPLETE;
    pls_busy = 0;
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

ISR (TIMER0_COMPA_vect)
{
	LED_PORT ^= _BV(LED_PIN);
}

/**
 * 初始化
 */
void pls_init(void)
{
  /*触发端口及外部中断0初始化*/
  SPARK_DDR &= ~_BV(SPARK_PIN);
  SPARK_PORT |= _BV(SPARK_PIN);
  EICRA = _BV(ISC00);
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
  OCR0A = 99U;
  TCNT0 = 0;

  /*定时器1CTC模式，OC1A管脚的状态产生预期脉冲*/
  TCCR1A = _BV(COM1A0);
//  TCCR1B = _BV(WGM12)|_BV(CS12)|_BV(CS11);
  TCCR1B = _BV(WGM12);
  TIFR1 = _BV(OCF1A);
  OCR1A = 5000U;
  TCNT1 = 0;  
  
  /*全局变量初始化*/
  pls_sta = PULSE_STA_COMPLETE;
  pls_mode = 0;
  pls_index = 0U;
  pls_busy = 0;
  pls_pre = 99U;
  delays = 5000U;
  widths = 5000U;
}

/**
 *@brief 手动设置延时、脉宽参数
  *@param[in] dly 预设置的延时数，单位0.1ms
  *@param[in] wtd 预设置的脉宽数，单位0.1ms
  *
  *延时数范围5000～99999，脉宽数范围4000～10000。仅在手动模式进行设置
  *@sa pls_set_param() 自动设置时间参数
 */
void pls_set_pulse(uint32_t dly,uint16_t wtd)
{
  if(0 != pls_mode )
  {
    if(dly < 65536UL)
    {
      delays = (uint16_t)dly;
      widths = wtd;
      if(delays < 5000U)
      {
        delays = 5000U;
      }
      pls_pre = 99U;
    }
    else if(dly < 100000UL)
    {
      delays = (uint16_t)(dly / 2);
      widths = wtd / 2;
      pls_pre = 199U;
    }
    else
    {
      delays = 49999UL;
      widths = wtd / 2;
      pls_pre = 199U;
    }
    if(pls_pre > 100U)
    {
    	if(widths > 5000U)
    	{
    		widths = 5000U;
    	}
    	else if(widths < 2000U)
    	{
    		widths = 2000U;
    	}
    	else
    	{
    		;
    	}
    }
    else
    {
    	if(widths > 10000U)
    	{
    		widths = 10000U;
    	}
    	else if(widths < 4000U)
    	{
    		widths = 4000U;
    	}
    	else
    	{
    		;
    	}
    }
  }
}

/**
 *@brief 设置工作模式
 *@param[in] mod 模式
 *-0自动模式，缺省值
 *-非零手动模式
 *@sa pls_get_mode() 取工作模式
 */
void pls_set_mode(uint8_t mod)
{
  pls_mode = mod;
}

/**
 *@brief 获取工作模式
 *@return 0自动模式，缺省值,非零手动模式
 *@sa pls_set_mode()  设置工作模式
 */
uint8_t pls_get_mode(void)
{
  return pls_mode;
}

/**
 *@brief 获取工作运行状态
 *@return 0未开始产生脉冲，缺省值；非零在进行，忙
 */
uint8_t pls_get_busy(void)
{
  return pls_busy;
}

/**
 *@brief 得到产生单脉冲的工作状态
 *@return 状态
 *- @ref PULSE_STA_DELAY 延时态，触发后首先进入的状态
 *- @ref PULSE_STA_WIDTH 脉宽态，经预定延时转换后的脉冲作用状态
 *- @ref PULSE_STA_COMPLETE 完成态，单脉冲产生已完成的状态或准备开始新的触发的状态
 *@sa pls_set_sta()   设置脉冲状态
 */
uint8_t pls_get_sta(void)
{
  return pls_sta;  
}

/**
 *@brief 设置工作状态
 *@sa pls_get_sta() 取脉冲状态
 */
void pls_set_sta(uint8_t sta)
{
  pls_sta = sta;  
}

/**
 *@brief 从FLASH存储数据进行设置延时、脉宽参数
 *@sa pls_set_pulse() 手动设置时间参数
 *@sa tims
 *@sa pls_index
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
  pls_busy = 0;
}

/**
 *@brief 得到延时数
 *@return 延时数，单位0.1ms
 */
uint32_t pls_get_delay(void)
{
  uint32_t ret;
  ret = delays;
  if(pls_pre > 100U)
  {
    ret *= 2U;
  }
  return ret;
}

/**
 *@brief 得到脉宽数
 *@return 脉宽数，单位0.1ms
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

/**
 *@brief 数字字符串转整型数
 *@param str 数字字符串
 *@return 整型数
 */
uint32_t pls_strtou(uint8_t str[])
{
  uint32_t ret = 0;
  int8_t i = 0;
  while(str[i] != 0)
  {
    ret *= 10U;
    ret += (uint8_t)(str[i++] - 0x30U);
    if(i >= 5U)
    {
      break;
    }
  }
  return ret;    
}
