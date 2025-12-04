#include "lcd.h"
#include "delay_us.h"
#include "delay_ms.h"
#include "serial.h"
//#include <stddef.h>
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef __SIZE_TYPE__ size_t;
#endif
#include <avr/pgmspace.h>

// --- Comandos internos del LCD ---
#define LCD_CMD_CLEAR           0x01
#define LCD_CMD_RETURN_HOME     0x02
#define LCD_CMD_ENTRY_MODE      0x06
#define LCD_CMD_DISPLAY_ON      0x0C
#define LCD_CMD_FUNCTION_SET    0x28 // 4-bit, 2 lineas, 5x7 dots
#define LCD_ADDR_ROW_0          0x80
#define LCD_ADDR_ROW_1          0xC0

// --- Funciones Privadas (Auxiliares) ---

static void lcd_pulse_enable(void) {
    gpio_pin(LCD_EN_PIN, 0); 
    delay_us(1);
    gpio_pin(LCD_EN_PIN, 1); 
    delay_us(1);            // Espera > 450ns
    gpio_pin(LCD_EN_PIN, 0);
    delay_us(100);          // Espera > 37us
}


static void lcd_write_nibble(uint8_t nibble) {
    gpio_pin(LCD_D4_PIN, (nibble) & 0x01);
    gpio_pin(LCD_D5_PIN, (nibble >> 1) & 0x01);
    gpio_pin(LCD_D6_PIN, (nibble >> 2) & 0x01);
    gpio_pin(LCD_D7_PIN, (nibble >> 3) & 0x01);
    
    lcd_pulse_enable();
}

static void lcd_send(uint8_t value, uint8_t mode) {
	
    // RS (0=Cmd, 1=Data)
    gpio_pin(LCD_RS_PIN, mode);
    
    //  RW a 0 (Escritura)
    gpio_pin(LCD_RW_PIN, 0);

    // (High Nibble)
    lcd_write_nibble(value >> 4);

    //  (Low Nibble)
    lcd_write_nibble(value);
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
	uint8_t address;
	if (row == 0) {
		address = LCD_ADDR_ROW_0 + col;
		} else {
		address = LCD_ADDR_ROW_1 + col;
	}
	
	lcd_send(address, 0); // 0 = Comando
}

// --- Funciones Públicas ---

void lcd_init(void) {
    gpio_output(LCD_RS_PIN);
    gpio_output(LCD_RW_PIN);
    gpio_output(LCD_EN_PIN);
    gpio_output(LCD_D4_PIN);
    gpio_output(LCD_D5_PIN);
    gpio_output(LCD_D6_PIN);
    gpio_output(LCD_D7_PIN);

    gpio_pin(LCD_RS_PIN, 0);
    gpio_pin(LCD_RW_PIN, 0);
    gpio_pin(LCD_EN_PIN, 0);

    // Espera > 15ms 
    delay_ms(50);

    // Secuencia para entrar en modo 4 bits
    
    gpio_pin(LCD_RS_PIN, 0);
    gpio_pin(LCD_RW_PIN, 0);

    lcd_write_nibble(0x03);
    delay_ms(10); // Espera > 4.1ms

    lcd_write_nibble(0x03);
    delay_us(150); // Espera > 100us

    lcd_write_nibble(0x03);
    
    lcd_write_nibble(0x02);


    // Configuración de funcionamiento
    lcd_send(LCD_CMD_FUNCTION_SET, 0); // 4-bit, 2 líneas, 5x7
    lcd_send(LCD_CMD_DISPLAY_ON, 0);   // Pantalla ON, Cursor OFF
    lcd_send(LCD_CMD_ENTRY_MODE, 0);   // Incremento automático derecha
    lcd_send(LCD_CMD_CLEAR, 0);        // Limpiar pantalla
    delay_ms(2); // Clear requiere > 1.52ms
}

void lcd_clear(void) {
    lcd_send(LCD_CMD_CLEAR, 0);
    delay_ms(2);
}

void lcd_print(char *str) {
    while (*str) {
        lcd_send((uint8_t)(*str), 1); // 1 = Dato (Caracter)
        str++;
    }
}


/* Función ligera para imprimir enteros sin usar sprintf */
void lcd_print_uint16(uint16_t value) {
	char buffer[6]; // Suficiente para 65535 (max valor posible de "value") + null 
	char *ptr = &buffer[5];
	*ptr = '\0';
	
	// Caso especial para 0
	if (value == 0) {
		*(--ptr) = '0';
		} else {
		while (value > 0) {
			*(--ptr) = (value % 10) + '0';
			value /= 10;
		}
	}
	lcd_print(ptr);
}

void lcd_print_flash(const char *str) {	
	uint8_t c;
	while ((c = pgm_read_byte(str++))) {
		lcd_send((uint8_t)c, 1); // 1 = Dato
	}
}