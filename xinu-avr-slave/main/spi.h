/*
 * spi.h
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

// --- Definición de Pines (Mapeo para gpio.c) ---
// En tu gpio.c, el Puerto B maneja los pines 8 a 13.
// PB2 = Pin 10 (SS)
// PB3 = Pin 11 (MOSI)
// PB4 = Pin 12 (MISO)
// PB5 = Pin 13 (SCK)
#define SPI_SS_PIN    10 
#define SPI_MOSI_PIN  11 
#define SPI_MISO_PIN  12 
#define SPI_SCK_PIN   13 

// --- Bits de Registros SPI (Máscaras) ---
#define SPE   6  // SPI Enable
#define MSTR  4  // Master/Slave Select (1 = Master)
#define CPOL  3  // Clock Polarity
#define CPHA  2  // Clock Phase
#define SPR1  1  // SPI Clock Rate Select 1
#define SPR0  0  // SPI Clock Rate Select 0

// SPSR (SPI Status Register)
#define SPIF  7  // SPI Interrupt Flag
#define SPI2X 0  // Double SPI Speed (SPI2X)

// Enum de Velocidades
typedef enum {
    SPI_CLOCK_DIV_4,   // 4 MHz (F_CPU / 4)
    SPI_CLOCK_DIV_16,  // 1 MHz (F_CPU / 16)
    SPI_CLOCK_DIV_64,  // 250 kHz (F_CPU / 64)
    SPI_CLOCK_DIV_128, // 125 kHz (F_CPU / 128) - Ideal para init de SD
    SPI_CLOCK_DIV_2,   // 8 MHz (F_CPU / 2) - Máxima velocidad
    SPI_CLOCK_DIV_8,   // 2 MHz (F_CPU / 8)
    SPI_CLOCK_DIV_32   // 500 kHz (F_CPU / 32)
} spi_clock_div_t;

// Prototipos
void spi_init(void);
void spi_set_speed(spi_clock_div_t clock_div);
unsigned char spi_transfer(unsigned char data);

// Funciones inline para uso rápido
static inline void spi_send(unsigned char data) {
    (void)spi_transfer(data);
}

static inline unsigned char spi_receive(void) {
    return spi_transfer(0xFF);
}

#endif /* SPI_H_ */