/*
 * led_matrix.c
 */

#include "led_matrix.h"
#include "gpio.h"
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef __SIZE_TYPE__ size_t;
#endif
#include <avr/pgmspace.h> // Necesario para PROGMEM

static const uint8_t ROW_PINS[4] PROGMEM = {ROW_0_PIN, ROW_1_PIN, ROW_2_PIN, ROW_3_PIN};
static const uint8_t COL_PINS[4] PROGMEM = {COL_0_PIN, COL_1_PIN, COL_2_PIN, COL_3_PIN};

// Helper para leer pin desde flash
static inline uint8_t get_row_pin(uint8_t i) { return pgm_read_byte(&ROW_PINS[i]); }
static inline uint8_t get_col_pin(uint8_t i) { return pgm_read_byte(&COL_PINS[i]); }

void matrix_init(void) {
    // Configurar todos los pines como SALIDA
    for (int i = 0; i < 4; i++) {
        gpio_output(ROW_PINS[i]);
        gpio_output(COL_PINS[i]);
    }
    matrix_clear();
}

void matrix_clear(void) {
    for (int i = 0; i < 4; i++) {
        gpio_pin(ROW_PINS[i], 0); // OFF
        gpio_pin(COL_PINS[i], 0); // OFF
    }
}

void matrix_render_frame(uint16_t state) {
	uint8_t r, c;
    // Itero sobre las 4 filas
    for (r = 0; r < 4; r++) {
        // Fila 0 son los bits más altos (shift 12), Fila 3 los más bajos (shift 0).
        // Shift = (3 - r) * 4
        uint8_t shift_amount = (3 - r) * 4;
        uint8_t row_data = (state >> shift_amount) & 0x0F;
        
        for (c = 0; c < 4; c++) {
			uint8_t pin = get_col_pin(c);
	        // Extraer bit sin variables extra
	        if ((row_data >> (3 - c)) & 0x01) {
		        gpio_pin(pin, 1);
		        } else {
		        gpio_pin(pin, 0);
	        }
        }
        
        // Encender fila
        gpio_pin(get_row_pin(r), 1);
		
        sleepms(3);
        
        // Apagar fila
        gpio_pin(get_row_pin(r), 0);
        
        // Limpiar columnas (Evitar ghosting)
        for (c = 0; c < 4; c++) {
	        gpio_pin(get_col_pin(c), 0);
        }
    }
}