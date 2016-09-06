/**
 * @brief 
 */
#ifndef UART_H
#define UART_H
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define UBRRH 	UBRR0H 
#define UBRRL 	UBRR0L
#define UDR		UDR0
#define UCSRA		UCSR0A
#define UCSRB		UCSR0B
#define UCSRC		UCSR0C
#define UDRE		UDRE0
#define RXC		RXC0
#define RXEN		RXEN0	 
#define TXEN		TXEN0	 
#define UCSZ1		UCSZ01	 
#define UCSZ0		UCSZ00	 
#define USBS		USBS0
#define FE			FE0
#define DOR		DOR0
#define U2X		U2X0
	 
#if defined(USE_PRINT)    
extern FILE uart_str;
#endif 

static inline void uart_init(uint32_t baud)
{
	uint16_t pri;
	if(baud<19200)	
	{
		pri = (int16_t)(F_CPU/(16*baud))-1;
	}
	else
	{
		pri = (int16_t)(F_CPU/(8*baud))-1;
		UCSRA |= _BV(U2X0);
	}		
	UBRRH = pri>>8;
	UBRRL =(uint8_t) pri;
	UCSRB = _BV(RXEN)|_BV(TXEN);
	UCSRC = _BV(UCSZ1)|_BV(UCSZ0);
#if defined(USE_PRINT)    
 stdout=stdin=&uart_str;
#endif 
}
static inline void uart_send(uint8_t byte)
{
	while(!(UCSRA&_BV(UDRE)));
	UDR = byte;
}
uint8_t uart_getchar(void);
uint8_t uart_getline(char str[],uint8_t n);
void uart_putsn(char str[],uint8_t n);
void uart_putsn_P(const __flash char str[],uint8_t n);
void uart_flush(void);
uint8_t uart_received(void);
#endif
