/**
 *@brief 5位数码管显示模块头文件
 *@file disp.h
 *@author shenxf 380406785@@qq.com
 *@version V1.1.0
 *@date	2016-10-17
 *函数列表：\n
 * @sa disp_init 初始化
 * @sa disp_on  开显示
 * @sa disp_off 关显示
 * @sa disp_play 将数传递至显示缓冲区
 * @sa disp_fill 填充字段码
 * @sa disp_filln 指定数位填充
*/
#ifndef DISP_H
#define DISP_H
#include <avr/io.h>

#define SEGC_POTR PORTC /**<a~f段，PC口*/
#define SEGC_DDR  DDRC /**<PC口方向*/
#define SEGD_PORT PORTD /**<g、dp段，PD口*/
#define SEGD_DDR  DDRD  /**<PD口方向*/
#define SEGD_PIN6 3     /**<g段管脚*/
#define SEGD_PIN7 4     /**<dp段管脚*/

#define DIGIT4_PORT   PORTD	/**<DS4第五位选端口，PD口*/
#define DIGIT4_DDR    DDRD	/**<DS4第五位选端口方向*/
#define DIGIT3_PORT   PORTB /**<DS3第四位选端口,PB口*/
#define DIGIT3_DDR    DDRB /**<DS3第四位选端口方向*/
#define DIGIT2_PORT   PORTB /**<DS2第三选端口,PB口*/
#define DIGIT2_DDR    DDRB /**<DS2第三位选端口方向*/
#define DIGIT1_PORT   PORTB /**<DS1第二位选端口，PB口*/
#define DIGIT1_DDR    DDRB /**<DS1第二位选端口方向*/
#define DIGIT0_PORT   PORTB /**<DS0第一位选端口，PB口*/
#define DIGIT0_DDR    DDRB /**<DS0第一位选端口方向*/
#define DIGIT_PIN4    7 /**<DS4第五位选端口管脚，PD7*/
#define DIGIT_PIN3    0 /**<DS3第四位选端口管脚，PB0*/
#define DIGIT_PIN2    2 /**<DS2第三位选端口管脚，PB2*/
#define DIGIT_PIN1    3 /**<DS1第二位选端口管脚，PB3*/
#define DIGIT_PIN0    4 /**<DS0第一位选端口管脚，PB4*/

#define DPOINT 0x80U  /**<dp小数点段编码权值*/

void disp_init(void);
void disp_on(void);
void disp_off(void);
void disp_play(uint32_t num);
void disp_fill(uint8_t segs);
void disp_filln(uint8_t segs,uint8_t digit);

#endif
