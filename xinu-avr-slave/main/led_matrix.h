/*
 * led_matrix.h
 */

#ifndef LED_MATRIX_H_
#define LED_MATRIX_H_

#include <stdint.h>

// Definición de pines según driver gpio.c
// PORTD
#define COL_0_PIN 2
#define COL_1_PIN 3
#define COL_2_PIN 4
#define COL_3_PIN 5

// PORTC (A0-A3 son 14-17 en gpio.c)
#define ROW_0_PIN 14
#define ROW_1_PIN 15
#define ROW_2_PIN 16
#define ROW_3_PIN 17

void matrix_init(void);

void matrix_render_frame(uint16_t state);

void matrix_clear(void);

#endif /* LED_MATRIX_H_ */