/**
 *@brief 单脉冲发生器
 *@file main.c
 *@author shenxf 380406785@@qq.com
 *@version V1.0.0
 *@date 2016-09-08
 *
 *软件运行硬件环境：atmega328p或Arduino nano、Arduino pro mini、Arduino UNO等开发板\n
 *软件编译环境：avr-gcc-4.8.1或更新的版本\n
 *主要功能：从INT0管脚响应一个触发信号，以这一信号触发定时器1开始计时，在预定的时间改变OC1A管脚电平状态\n
 *产生预定的单脉冲信号。单脉冲发生器设置两个工作模式：自动模式和手动模式。自动模式是根据触发信号的输入\n
 *自动产生不同的单脉冲信号；手动模式是通过串口的人机交互手动产生单脉冲信号。单脉冲的时间参数由七段数码管\n
  *显示器进行显示
*/
#include <avr/interrupt.h>
#include <util/delay.h>
#include "pulse.h"
#include "disp.h"
#include "uart.h"

/**
 *@var __flash const char prompt[80]
 *@brief 存在FLASH的提示字符串
*/
__flash const char prompt[80]=
"\n\rPress 'm' or 'M' enter manulation model\n\rPress 'a' or 'A' return auto model";

/**
 *@var __flash const char pdelay[40]
 *@brief 存在FLASH的延时输入提示字符串
*/
__flash const char pdelay[40] = "\n\rDelay number(5000-99999) unit 0.1ms:";

/**
 *@var __flash const char pwidth[40]
 *@brief 存在FLASH的脉宽输入提示字符串
*/
__flash const char pwidth[40] = "\n\rWidth number(4000-10000) unit 0.1ms:";

/**
 *@var __flash const char pstart[20]
 *@brief 存在FLASH的开始输出脉冲提示字符串
*/
__flash const char pstart[20] = "\n\rStart generate";

/**
 *@var __flash const char psucc[8]
 *@brief 存在FLASH的输出脉冲完成提示字符串
*/
__flash const char psucc[8] = "\n\rSucc";

/**
 *@var __flash const char pnready[12]
 *@brief 存在FLASH的准备接收触发提示字符串
*/
__flash const char pnready[12] = "\n\rNo ready";

/**
 *@var __flash const char pwaitting[16]
 *@brief 存在FLASH的等待输出脉冲完毕提示字符串
*/
__flash const char pwaitting[16] = "\n\rWaitting...";

/**
 *@brief IO口上电初始化
 *
 *系统上电复位程序入口，初始化所有外部接口，然后再系统初始化。避免由于Arduino等开发板的Bootloader已\n
 *对IO进行了初始化，影响本系统。
*/
void init_portb (void) __attribute__ ((naked)) __attribute__ ((section (".init1")));
void init_portb (void)
{
  PORTB = 0x00;
  DDRB = 0x00;
  PORTC = 0x00;
  DDRC = 0x00;
  PORTD = 0x00;
  DDRD = 0x00;
}

/**
 *@brief 主函数
 *
 * 主控制函数，
*/
int main(void)
{
  uint8_t ch;/*串口接收字符*/
  uint8_t ind = 0; /*循环控制变量*/
  uint32_t udelay; /**/
  uint16_t uwidth; /**/
  uint8_t strnum[8];/**/

  /*各模块初始化，波特率115200，开总中断*/
  pls_init();
  disp_init();
  uart_init(115200UL);
  sei();

  /*闪亮显示“8.8.8.8.8."*/
  disp_fill(0xff);
  for(ind = 0;ind < 3;ind++)
  {
    disp_on();
    _delay_ms(1000);
    disp_off();
    _delay_ms(1000);
  }

  /*显示“12345”和“67890”*/
  disp_play(12345);
  disp_on();
  _delay_ms(1000);
  disp_play(67890);
  _delay_ms(1000);

  /*点亮状态指示灯，发送串口提示*/
  ind = 0;
  LED_PORT |= _BV(LED_PIN);
  uart_putsn_P(prompt,80);
  
  /*主控制流程*/
  while(1)
  {
    if(pls_get_mode() == 0)
    {
      /*自动模式控制*/
      if(SPARK_PINS & _BV(SPARK_PIN))
      {
        _delay_ms(20);
        if(SPARK_PINS & _BV(SPARK_PIN))
        {
          /*触发端口状态正常，熄灭指示灯，关显示，获取预产生的延时和脉宽参数*/
          disp_off();
          LED_PORT &= ~_BV(LED_PIN);
          pls_set_param();
          pls_set_sta(PULSE_STA_DELAY );
          
          /*准备响应触发，显示时间参数*/
          EIMSK |= _BV(INT0);
          disp_on();
          disp_play(pls_get_delay());

          /*等待单脉冲输出完成*/
          while(pls_get_sta() != PULSE_STA_COMPLETE)
          {
            /*等待过程中交替显示时间参数，约 0.5秒显示延时和脉宽，闪亮指示灯*/
            if(0 == ind)
            {
              disp_play(pls_get_delay());
              LED_PORT |= _BV(LED_PIN);
            }
            if( 50U == ind)
            {
              disp_play(pls_get_width());
              LED_PORT &= ~_BV(LED_PIN);
            }
            if(100 == ind)
            {
              ind = 0;
            }
            else
            { 
              ind++;
            }    
		    _delay_ms(10);		 
          }
          
          /*单脉冲输出完毕，显示“End”*/
          EIMSK = 0;
          disp_fill(0);
          disp_filln(0x79,2);
          disp_filln(0x54,1);
          disp_filln(0x5e,0);

          /*1秒内若接收到手动模式命令，将进入手动模式*/
          ind = 0;
          do
          {
            if(uart_received() != 0)
            {
              LED_PORT &= ~_BV(LED_PIN);
              ch = uart_getchar();
              if(('m' == ch)||('M' == ch))
              {
                pls_set_mode(1U);
                break;
              }
              _delay_ms(10);
              ind++;
            }
          }  
          while(ind <= 100U);
          ind = 0;
        }
      }
      else
      {
        /*触发端口电平状态异常*/
        /*闪烁显示“-----“及指示灯*/
        if( 0 == ind)
        {
          disp_fill(0x40U);
          LED_PORT |= _BV(LED_PIN);
        }
        if(50 == ind)
        {
          LED_PORT &= ~_BV(LED_PIN);
          disp_off();
        }
        if (ind >= 100U)
        {
          ind = 0;
        }
        else
        {
          ind++;
        }
        
          /*接收到手动模式命令，将进入手动模式*/
        if(uart_received() != 0)
        {
          LED_PORT &= ~_BV(LED_PIN);
          ch = uart_getchar();
          if(('m' == ch)||('M' == ch))
          {
            pls_set_mode(1U);
          }
        }
        _delay_ms(10);
      }
    }
    else
    {
      /*手动模式*/
      if(SPARK_PINS & _BV(SPARK_PIN))
      {
        _delay_ms(20);
        if(SPARK_PINS & _BV(SPARK_PIN))
        {
          /*触发端口状态正常，关显示，接收延时数*/
          disp_off();
          uart_putsn_P(pdelay,40);
          uart_getnum(strnum);
          udelay = pls_strtou(strnum);

          /*接收脉宽数，设置时间参数*/
          uart_putsn_P(pwidth,40);
          uart_getnum(strnum);
          uwidth = (uint16_t)pls_strtou(strnum);
          pls_set_pulse(udelay,uwidth);
          
          /*准备响应触发，显示时间参数*/
          uart_putsn_P(pstart,20);
          EIMSK |= _BV(INT0);
          disp_on();
          disp_play(pls_get_delay());
          
          /*等待单脉冲输出完成*/          
          while(pls_get_sta() != PULSE_STA_COMPLETE)
          {
            /*等待过程中交替显示时间参数，约 0.5秒显示延时和脉宽，闪亮指示灯*/
            if(0 == ind)
            {
              disp_play(pls_get_delay());
              LED_PORT |= _BV(LED_PIN);
              uart_putsn_P(pwaitting,16);
            }
            if( 50U == ind)
            {
              disp_play(pls_get_width());
              LED_PORT &= ~_BV(LED_PIN);
            }
            if(100 == ind)
            {
              ind = 0;
            }
            else
            { 
              ind++;
            }    
		    _delay_ms(10);		 
          }
          
          /*单脉冲输出完毕，显示“End”*/
          uart_putsn_P(psucc,8);
          EIMSK = 0;
          disp_fill(0);
          disp_filln(0x79,2);
          disp_filln(0x54,1);
          disp_filln(0x5e,0);
          ind = 0;

          /*1秒内若接收到自动模式命令，将进入自动模式*/
          do
          {
            if(uart_received() != 0)
            {
              LED_PORT &= ~_BV(LED_PIN);
              ch = uart_getchar();
              if(('a' == ch)||('A' == ch))
              {
                pls_set_mode(0U);
                break;
              }
              _delay_ms(10);
              ind++;
            }
          }  
          while(ind <= 100U);
          ind = 0;          
        }
      }
      else
      {
        /*触发端口电平状态异常*/
        /*闪烁显示“-----“及指示灯*/
        if( 0 == ind)
        {
          disp_fill(0x40U);
          LED_PORT |= _BV(LED_PIN);
          uart_putsn_P(pnready,12);
        }
        if(50 == ind)
        {
          LED_PORT &= ~_BV(LED_PIN);
          disp_off();
        }
        if (ind >= 100U)
        {
          ind = 0;
        }
        else
        {
          ind++;
        }

          /*接收到自动模式命令，将进入自动模式*/
        if(uart_received() != 0)
        {
         LED_PORT &= ~_BV(LED_PIN);
         ch = uart_getchar();
         if(('m' == ch)||('M' == ch))
         {
            pls_set_mode(1U);
         }
        }
        _delay_ms(10);
      }
    }
  }
  return 0;
}
