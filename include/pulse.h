/**
 * @brief 单脉冲源头文件
 * @file
 * @author shenxf 380406785@@qq.com
 * @version V1.2.0
 * @date 2016-10-24
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
#ifndef PULSE_H
#define PULSE_H
#include <avr/io.h>
#include <stdint.h>

#define SPARK_PORT  PORTD /**<触发端口，PD口*/
#define SPARK_DDR   DDRD  /**<触发端口方向，输入*/
#define SPARK_PINS  PIND  /**<触发端口输入寄存器*/
#define SPARK_PIN   2     /**<触发端口位，PD2脚，Aeduino Nano D2*/

#define CLKOUT_PORT  PORTD  /**<时基输出端口，PD口*/
#define CLKOUT_DDR   DDRD   /**<时基输出方向，输出*/
#define CLKOUT_PIN   6      /**<时基输出管脚，PD6脚，Aeduino Nano D6*/

#define CLKIN_PORT  PORTD   /**<时基输入端口，PD口*/
#define CLKIN_DDR   DDRD    /**<时基输入方向，输入*/
#define CLKIN_PIN   5       /**<时基输入管脚，PD5脚，Aeduino Nano D5*/

#define LED_PORT    PORTB   /**<指示灯输出端口，PB口*/
#define LED_DDR     DDRB    /**<指示灯输出方向，输出*/
#define LED_PIN     5       /**<指示灯输出管脚，PB5脚，Aeduino Nano D13*/

#define PULSE_PORT  PORTB   /**<脉冲输出端口，PB口*/
#define PULSE_DDR   DDRB    /**<脉冲输出方向，输出*/
#define PULSE_PIN   1       /**<脉冲输出管脚，PB1脚，Aeduino Nano D9*/

#define PULSE_STA_DELAY       0x00U   /**<脉冲波的延迟态*/
#define PULSE_STA_WIDTH       0x01U   /**<脉冲波的宽度态*/
#define PULSE_STA_COMPLETE    0x02U   /**<脉冲波的完成态*/

void pls_init(void);
void pls_set_pulse(uint32_t dly,uint16_t wtd);
void pls_set_mode(uint8_t mod);
void pls_set_param(void);
void pls_set_sta(uint8_t sta);
uint8_t pls_get_sta(void);
uint8_t pls_get_busy(void);
uint8_t pls_get_mode(void);
uint32_t pls_get_delay(void);
uint16_t pls_get_width(void);
uint32_t pls_strtou(uint8_t str[]);
#endif
