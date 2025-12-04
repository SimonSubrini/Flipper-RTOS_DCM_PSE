/*
 * i2c.c
 */

#include "i2c.h"

typedef struct
{
  uint8_t twbr; // Bit Rate Register
  uint8_t twsr; // Status Register
  uint8_t twar; // Address Register (No usado en Master I2C)
  uint8_t twdr; // Data Register
  uint8_t twcr; // Control Register
} volatile i2c_t;

// Puntero a la dirección base (0xB8)
volatile i2c_t *i2c = (i2c_t *) 0xB8;

/* --- Definición de Bits (Privados del Driver) --- */
#define TWINT 7 // Interrupt Flag
#define TWEA  6 // Enable Acknowledge
#define TWSTA 5 // START Condition
#define TWSTO 4 // STOP Condition
#define TWEN  2 // Enable TWI

/* --- Funciones Privadas --- */

// Espera bloqueante 
static void i2c_wait(void) {
    // Espera hasta que el flag TWINT se ponga a 1
    while (!(i2c->twcr & (1 << TWINT)));
}

/* --- Funciones Públicas --- */

void i2c_init(void) {
    // Configurar SCL frequency a 400kHz (Fast Mode)
    // Formula: SCL_freq = F_CPU / (16 + 2 * TWBR * Prescaler)
    // Para 400kHz @ 16MHz:
    // 400k = 16M / (16 + 2 * TWBR * 1)
    // 2 * TWBR = 24 -> TWBR = 12
    
    i2c->twsr = 0x00; // Prescaler = 1
    i2c->twbr = 12;   // Bit Rate value
    
    // Habilitar el módulo TWI
    i2c->twcr = (1 << TWEN);
}

void i2c_start(void) {
    // Enviar condición START
    // Se debe escribir 1 en TWINT para limpiar la bandera e iniciar
    i2c->twcr = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    i2c_wait();
}

void i2c_stop(void) {
    // Enviar condición STOP
    i2c->twcr = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void i2c_write(uint8_t data) {
    // Cargar dato
    i2c->twdr = data;
    
    // Iniciar transmisión (Limpiar TWINT, Habilitar TWI)
    i2c->twcr = (1 << TWINT) | (1 << TWEN);
    
    // Esperar a que el byte salga
    i2c_wait();
}