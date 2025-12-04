  
/*
 * driver gpio: permite operar bits de los puertos gpios individualmente,
 * o en forma paralela (con los 8 bits del puerto).
 */

#include "gpio.h"

/*
 * Ejemplos de uso:
 *
 * // salida
 * gpio_output(13);
 * gpio_pin(13, ON);
 * gpio_pin(13, OFF);
 * gpio_pin(13, TOGGLE);
 *
 * // entrada
 * unsigned char v;
 * gpio_input(13);
 * v = gpio_pin(13, GET);
 *
 * // activar pull up
 * gpio_input(13);
 * gpio_pin(13, ON);
 
/* gpio_pin(): opera sobre un pin individual (en salida, activar pull-up, o 
 * leyendo su valor de entrada).
 *
 * Argumentos:
 * pin: etiqueta del pin en la placa (ejemplo: 13)
 * op es la operación: ON, OFF, TOGGLE o GET
 *
 */

#define PINB  ((volatile unsigned char *)0x23)
#define DDRB  ((volatile unsigned char *)0x24)
#define PORTB ((volatile unsigned char *)0x25)

#define PINC  ((volatile unsigned char *)0x26)
#define DDRC  ((volatile unsigned char *)0x27)
#define PORTC ((volatile unsigned char *)0x28)

#define PIND  ((volatile unsigned char *)0x29)
#define DDRD  ((volatile unsigned char *)0x2A)
#define PORTD ((volatile unsigned char *)0x2B)

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
	        *DDRC &= ~(1 << (p - 14));
            break;
        }
}
