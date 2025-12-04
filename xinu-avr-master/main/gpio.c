  // Script desarrollado en la cursada de PSE - adaptado para incluir interrupciones via INT0
/*
 * driver gpio: permite operar bits de los puertos gpios individualmente,
 * o en forma paralela (con los 8 bits del puerto).
 */

#include "gpio.h"
#include <avr/interrupt.h>

/* --- Definición manual de NULL --- */
#ifndef NULL
#define NULL ((void *)0)
#endif

#define PINB  ((volatile unsigned char *)0x23)
#define DDRB  ((volatile unsigned char *)0x24)
#define PORTB ((volatile unsigned char *)0x25)

#define PINC  ((volatile unsigned char *)0x26)
#define DDRC  ((volatile unsigned char *)0x27)
#define PORTC ((volatile unsigned char *)0x28)

#define PIND  ((volatile unsigned char *)0x29)
#define DDRD  ((volatile unsigned char *)0x2A)
#define PORTD ((volatile unsigned char *)0x2B)

/* --- Registros de Interrupción Externa (ATmega328p) --- */
// EICRA (External Interrupt Control Register A): Configura flancos (0x69)
#define EICRA (*(volatile unsigned char *)0x69)
// EIMSK (External Interrupt Mask Register): Habilita interrupciones (0x3D)
#define EIMSK (*(volatile unsigned char *)0x3D)

/* --- Variable Privada para Callback --- */
static void (*int0_callback_ptr)(void) = NULL;

int gpio_pin(int p, int op) 
{
        unsigned char value=0;
        // Lógica para detectar puerto (0=D, 1=B, 2=C)
        unsigned char reg = (p < 8)? 0 : (p < 14)? 1 : 2;    
        switch(reg){
	        case 0: // PORT D
				if (op == 0)      *PORTD &= ~(1 << p);
				else if (op == 1) *PORTD |= (1 << p);
				else if (op == 2) *PORTD ^= (1 << p);
				else              value = (*PIND >> p) & 0x01;
				break;

	        case 1: // PORT B
				if (op == 0)      *PORTB &= ~(1 << (p - 8));
				else if (op == 1) *PORTB |= (1 << (p - 8));
				else if (op == 2) *PORTB ^= (1 << (p - 8));
				else              value = (*PINB >> (p - 8)) & 0x01;
				break;

	        default: // PORT C
				if (op == 0)      *PORTC &= ~(1 << (p - 14));
				else if (op == 1) *PORTC |= (1 << (p - 14));
				else if (op == 2) *PORTC ^= (1 << (p - 14));
				else              value = (*PINC >> (p - 14)) & 0x01;
				break;
        }
        
        return value;
}


/* establece el pin p (o puerto p) como entrada */
void gpio_input(int p)
{
	// Lógica para detectar puerto (0=D, 1=B, 2=C)
	unsigned char reg = (p < 8)? 0 : (p < 14)? 1 : 2;
    switch(reg){
        case 0:
	        *PORTD &= ~(1 << p); 
	        *DDRD &= ~(1 << p);
            break;
        case 1:
	        *PORTB &= ~(1 << (p - 8)); 
	        *DDRB &= ~(1 << (p - 8));
            break;
        default:
	        *PORTC &= ~(1 << (p - 14)); 
	        *DDRC &= ~(1 << (p - 14));
            break;
        }
}

/* establece el pin p (o puerto p) como salida */
void gpio_output(int p)
{
	// Lógica para detectar puerto (0=D, 1=B, 2=C)
	unsigned char reg = (p < 8)? 0 : (p < 14)? 1 : 2;
    switch(reg){
        case 0:
	        *PORTD &= ~(1 << p); 
	        *DDRD |=  (1 << p); 
            break;
        case 1:
	        *PORTB &= ~(1 << (p - 8)); 
	        *DDRB |=  (1 << (p - 8)); 
             break;
        default:
	        *PORTC &= ~(1 << (p - 14)); 
	        *DDRC |=  (1 << (p - 14)); 
            break;
        }
}

void gpio_attach_int0(uint8_t mode, void (*callback)(void)) {
	int0_callback_ptr = callback;

	// Configuro el pin (INT0 es Pin 2) como entrada
	gpio_input(2);
	gpio_pin(2, ON); // activo pull-up interno

	// Configuro el modo de disparo en EICRA
	// Los bits de INT0 son ISC01 (bit 1) e ISC00 (bit 0)
	// Limpio los bits primero (mascara 0xFC = 11111100)
	EICRA &= 0xFC;
	// Seteo el modo 
	EICRA |= (mode & 0x03);

	// Habilito la interrupción en EIMSK
	// Bit INT0 es el bit 0
	EIMSK |= 0x01;
}

ISR(INT0_vect) {
	if (int0_callback_ptr != NULL) {
		int0_callback_ptr();
	}
}