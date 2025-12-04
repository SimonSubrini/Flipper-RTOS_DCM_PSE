/*
 * timer1.c - Driver del TIMER1 del atmega328p
 */

#include "timer1.h"
#include <stdint.h> 
//#include <stddef.h> // Para NULL
#ifndef NULL
#define NULL ((void *)0)
#endif
#include <avr/interrupt.h>

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
volatile unsigned  char *timer1_timsk1 = (unsigned char *) 0x6F;

// Variable privada para guardar el puntero a la función de audio
static void (*timer1_callback_ptr)(void) = NULL;

// --- Configuración Hardcodeada (Migrada de tu main.c) ---
// TCCR1B: WGM12 (bit 3) = 1 (CTC Mode), CS10 (bit 0) = 1 (No Prescaler)
#define CONF_TCCR1B  ((1 << 3) | (1 << 0)) 
// TIMSK1: OCIE1A (bit 1)
#define CONF_TIMSK1_ENABLE  (1 << 1)

// Valores de Comparación (0x0B55)
#define VAL_OCR1AH   0x0B
#define VAL_OCR1AL   0x55

void timer1_init(void (*isr_callback)(void))
{
    // Guardar el callback de la aplicación
    timer1_callback_ptr = isr_callback;

    // Configurar Registros (Apagado inicialmente)
    timer1->tccr1a = 0;              // Modo Normal port operation
    timer1->tccr1b = CONF_TCCR1B;    // CTC + No Prescaler
    
    // Cargar valor de comparación (High byte first siempre en 16-bit)
    timer1->ocr1ah = VAL_OCR1AH;
    timer1->ocr1al = VAL_OCR1AL;
    
    // Asegurar interrupción apagada al inicio
    *timer1_timsk1 &= ~CONF_TIMSK1_ENABLE;
}

void timer1_start(void)
{
    // Habilitar interrupción OCIE1A
    *timer1_timsk1 |= CONF_TIMSK1_ENABLE;
}

void timer1_stop(void)
{
    // Deshabilitar interrupción OCIE1A
    *timer1_timsk1 &= ~CONF_TIMSK1_ENABLE;
}

// --- ISR Driver ---
// La interrupción delega el trabajo a main.c
ISR(TIMER1_COMPA_vect)
{
    if (timer1_callback_ptr != NULL) {
        timer1_callback_ptr(); // Ejecutar lógica de audio
    }
}