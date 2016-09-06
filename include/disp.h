#ifndef DISP_H
#define DISP_H
#include <avr/io.h>

#define SEGC_POTR PORTC
#define SEGC_DDR  DDRC
#define SEGD_PORT PORTD
#define SEGD_DDR  DDRD
#define SEGD_PIN6 3
#define SEGD_PIN7 4

#define DIGIT4_PORT   PORTD
#define DIGIT4_DDR    DDRD
#define DIGIT3_PORT   PORTB
#define DIGIT3_DDR    DDRB
#define DIGIT2_PORT   PORTB
#define DIGIT2_DDR    DDRB
#define DIGIT1_PORT   PORTB
#define DIGIT1_DDR    DDRB
#define DIGIT0_PORT   PORTB
#define DIGIT0_DDR    DDRB
#define DIGIT_PIN4    7
#define DIGIT_PIN3    0
#define DIGIT_PIN2    2
#define DIGIT_PIN1    3
#define DIGIT_PIN0    4

#define DPOINT 0x80U

void disp_init(void);
void disp_on(void);
void disp_off(void);
void disp_play(uint32_t num);
void disp_fill(uint8_t segs);
void disp_filln(uint8_t segs,uint8_t digit);

#endif
