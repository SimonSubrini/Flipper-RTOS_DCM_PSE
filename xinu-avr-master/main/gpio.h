/*
 * gpio.h - Driver GPIO e Interrupciones Externas
 */

#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

/* --- Constantes GPIO --- */
#define OFF     0x0
#define ON      0x1
#define TOGGLE  0x2
#define GET     0x3

/* --- Identificadores de Puerto (para funciones internas) --- */
#define PORT_B  0x4
#define PORT_C  0x5
#define PORT_D  0x6

/* --- Mapeo de Pines Analógicos --- */
#define A0 0x0E
#define A1 0x0F
#define A2 0x10
#define A3 0x11
#define A4 0x12
#define A5 0x13
#define A6 0x14
#define A7 0x15

/* --- Modos de Interrupción (INT0) --- */
// Valores basados en bits ISC01 e ISC00 del registro EICRA
#define INT_LOW_LEVEL    0x0 // 00
#define INT_ANY_CHANGE   0x1 // 01
#define INT_FALLING_EDGE 0x2 // 10
#define INT_RISING_EDGE  0x3 // 11


int gpio_pin(int p, int op);
void gpio_input(int p);
void gpio_output(int p);
void gpio_attach_int0(uint8_t mode, void (*callback)(void));

#endif /* GPIO_H */