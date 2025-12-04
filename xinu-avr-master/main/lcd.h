/*
 * lcd.h
 * Driver para LCD 16x2 en modo 4-bits.
 *
 * Requiere: RS, RW, EN y D4-D7.
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include "gpio.h"

// --- Configuración de Pines (según mapeo Arduino Uno / gpio.c) ---
// RS, RW, EN en Puerto C (14,15,16)
#define LCD_RS_PIN  A0
#define LCD_RW_PIN  A1
#define LCD_EN_PIN  A2

// Bus de Datos en Puerto D (4, 5, 6, 7)
#define LCD_D4_PIN  4
#define LCD_D5_PIN  5
#define LCD_D6_PIN  6
#define LCD_D7_PIN  7

void lcd_init(void);

void lcd_clear(void);

void lcd_set_cursor(uint8_t, uint8_t);

void lcd_print(char *str);

void lcd_print_uint16(uint16_t);

void lcd_print_flash(const char *str);

#endif /* LCD_H_ */