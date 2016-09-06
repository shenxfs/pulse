#include <avr/interrupt.h>
#include <util/delay.h>
#include "pulse.h"
#include "disp.h"
#include "uart.h"

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

int main(void)
{
  uint8_t i;
  uint8_t idl = 0;
  pls_init();
  disp_init();
  uart_init(115200UL);
  sei();
  disp_fill(0xff);
  for(i = 0;i < 3;i++)
  {
    disp_on();
    _delay_ms(1000);
    disp_off();
    _delay_ms(1000);
  }
  disp_play(12345);
  disp_on();
  _delay_ms(1000);
  i = 0;
  LED_PORT |= _BV(LED_PIN);
  while(1)
  {
      if(_BV(SPARK_PIN) == (SPARK_PINS & SPARK_PIN))
      {
        idl = 0;
      }
      else
      {
        if( 0 == idl)
        {
          disp_fill(0x40U);
          idl = 1U;
        }
        if(50 == i)
        {
          LED_PORT &= ~_BV(LED_PIN);
          disp_off();
        }
        else if (i >= 100U)
        {
          LED_PORT |= _BV(LED_PIN);
          disp_on();
          i = 0;
        }
        else
        {
          ;/*no deal with*/
        }
        i++;
        _delay_ms(10);
      }
  }
  return 0;
}
