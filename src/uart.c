/**
 * @brief
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"

uint8_t uart_rxbuf[16];
volatile uint8_t uart_head;
volatile uint8_t uart_end;


ISR(USART_RX_vect)
{
  uint8_t ind;
  ind = uart_end;
  uart_rxbuf[ind] = UDR;
  ind++;
  if(ind >= 16U)
  {
    ind = 0;
  }
  uart_end = ind;
}

void uart_flush(void)
{
  uart_head = 0;
  uart_end = 0;
}

uint8_t uart_received(void)
{
  return (uint8_t)(uart_head != uart_end);
}

void uart_putsn(char str[],uint8_t n)
{
	uint8_t i;
    uint8_t ch;
    for(i = 0;i < n;i++)      
	{
      ch = (uint8_t)str[i];
      if('\n' == ch)
      {
        uart_send('\r');					
      }
      else if('\0' == ch)
      {
        break;
      }
      else
      {
        uart_send(ch);
      }
	}
}

void uart_putsn_P(const __flash char str[],uint8_t n)
{
	uint8_t i;
    uint8_t ch;
    for(i = 0;i < n;i++)      
	{
      ch = (uint8_t)str[i];
      if('\n' == ch)
      {
        uart_send('\r');					
      }
      else if('\0' == ch)
      {
        break;
      }
      else
      {
        uart_send(ch);
      }
	}
}
