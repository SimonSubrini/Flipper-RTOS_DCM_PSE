/*
 * timer1.c - Driver del TIMER1 para control de servos
 */

#include "timer1_pwm.h"
#include "gpio.h"

// --- Estructura de Registros ---
typedef struct
{
  uint8_t tccr1a;
  uint8_t tccr1b;
  uint8_t tccr1c;
  uint8_t reserved;
  uint8_t tcnt1l;
  uint8_t tcnt1h;
  uint8_t icr1l;
  uint8_t icr1h;
  uint8_t ocr1al;
  uint8_t ocr1ah;
  uint8_t ocr1bl;
  uint8_t ocr1bh;
} volatile timer1_t;

// Punteros a direcciones físicas de memoria
volatile timer1_t *timer1 = (timer1_t *) 0x80;

/* --- Definición de Bits Clave --- */
// TCCR1A
#define COM1A1 7
#define COM1B1 5
#define WGM11  1
// TCCR1B
#define WGM13  4
#define WGM12  3
#define CS11   1 // Prescaler 8



void timer1_init(void)
{
	// OC1A (Pin 9) y OC1B (Pin 10) como SALIDA para que el PWM 
	gpio_output(SERVO_PIN_A);
	gpio_output(SERVO_PIN_B);
	
    // Configurar Registros 
	// Modo 14 (Fast PWM, TOP=ICR1)
	// Prescaler 8 (0.5us por tick a 16MHz)
	// Salida No Invertida (Clear on Compare Match)
    timer1->tccr1a = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    timer1->tccr1b = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    
	// Establezco Frecuencia (50Hz = 20ms)
	// 20ms / 0.5us = 40,000 ticks
	uint16_t ticks = 39999; // pongo un tick menos porque el contador arranca en 0
	timer1->icr1h = (ticks >> 8);
	timer1->icr1l = (ticks & 0xFF);

	// Inicializar en extremo inferior
	timer1_pwm_set_duty(1, 4000); // 4000 pulsos de 0.5us = 2ms
	timer1_pwm_set_duty(2, 4000); // Rango valido de valores 2000 - 4000
	
}

void timer1_pwm_set_duty(uint8_t channel, uint16_t ticks) {
	if (channel == 1) {
		timer1->ocr1ah = (ticks >> 8);
		timer1->ocr1al = (ticks & 0xFF);
	} else if (channel == 2) {
		timer1->ocr1bh = (ticks >> 8);
		timer1->ocr1bl = (ticks & 0xFF);
	}
}