// Script desarrollado en la cursada de PSE
/**********************************************************************
 *
 * serial.c - Driver del UART del atmega328p
 *
 * Configuracion: 9600bps, 8bits data, 1bit stop, sin bit de paridad
 *
 **********************************************************************/

#include <stdint.h>
//#include <stddef.h>
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef __SIZE_TYPE__ size_t;
#endif
#include <avr/pgmspace.h>

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE  (((F_CPU/(USART_BAUDRATE*16UL)))-1)

/* UCSR0A - USART Control and Status Register A bits */
#define UCSR0A_MPCM0 0
#define UCSR0A_U2X0 1
#define UCSR0A_UPE0 2
#define UCSR0A_DOR0 3
#define UCSR0A_FE0 4
#define UCSR0A_UDRE0 5
#define UCSR0A_TXC0 6
#define UCSR0A_RXC0 7

/* UCSR0B - USART Control and Status Register B bits */
#define UCSR0B_TXB80 0
#define UCSR0B_RXB80 1
#define UCSR0B_UCSZ02 2
#define UCSR0B_TXEN0 3
#define UCSR0B_RXEN0 4
#define UCSR0B_UDRIE0 5
#define UCSR0B_TXCIE0 6
#define UCSR0B_RXCIE0 7

/* UCSR0C - USART Control and Status Register C bits */
#define UCSR0C_UCPOL0 0
#define UCSR0C_UCSZ00 1
#define UCSR0C_UCSZ01 2
#define UCSR0C_USBS0 3
#define UCSR0C_UPM00 4
#define UCSR0C_UPM01 5
#define UCSR0C_UMSEL00 6
#define UCSR0C_UMSEL01 7

#define RX_E (1 << UCSR0B_RXEN0)		   /* Habilitar la recepción */
#define TX_E (1 << UCSR0B_TXEN0)		   /* Habilitar la transmisión */
#define PARITY_MODE (0 << UCSR0C_UPM00)	   /* No usar bit de paridad */
#define STOP_BITS (0 << UCSR0C_USBS0)	   /* Usar 1 bit de stop */
#define CHAR_SIZE (3 << UCSR0C_UCSZ00)	   /* Usar 8 bits de dato de E/S */
#define READY_TO_READ (1 << UCSR0A_RXC0)   /* Dato listo para leer */
#define READY_TO_WRITE (1 << UCSR0A_UDRE0) /* Buffer listo para escribir */

#define MAX_INT_DIGITS 5
#define MAX_LONG_DIGITS 10
#define MAX_DOUBLE_PRECISION 10

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* Estructura de datos overlay del USART del atmega328 */
typedef struct {
	uint8_t status_control_a; /* ucsr0a USART Control and Status A */
	uint8_t status_control_b; /* ucsr0b USART Control and Status B */
	uint8_t status_control_c; /* ucsr0c USART Control and Status C */
	uint8_t _reserved;        /* reserved */
	uint8_t baud_rate_l;      /* ubrr0l baud rate low */;
	uint8_t baud_rate_h;      /* ubrr0h baud rate high */
	uint8_t data_io;          /* udr0 I/O data */
} volatile uart_t;

/* Puntero a la estructura de los registros del periférico */
uart_t *serial_port = (uart_t*)(0xc0);

void serial_init(void)
{
	/* Configurar los registros High y Low con BAUD_PRESCALE */
	serial_port->baud_rate_h = (unsigned char)(BAUD_PRESCALE >> 8);
	serial_port->baud_rate_l = (unsigned char)(BAUD_PRESCALE);

	/* Configurar un frame de 8bits, con un bit de paridad y bit de stop */
	serial_port->status_control_c = CHAR_SIZE | STOP_BITS | PARITY_MODE;

	/* Activar la recepcion y transmicion */
	serial_port->status_control_b = RX_E | TX_E;
}

char serial_get_char(void)
{
	while (!((serial_port->status_control_a) & (READY_TO_READ)))
		;
	return (serial_port->data_io);
}

void serial_put_str_flash(const char *str) {
	char c;
	while ((c = pgm_read_byte(str++))) { // Lee byte por byte desde la Flash
		while (!(serial_port->status_control_a & READY_TO_WRITE))
		;
		serial_port->data_io = c;
	}
}