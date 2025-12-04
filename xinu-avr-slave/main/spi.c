/*
 * spi.c
 */

#include "spi.h"
#include "gpio.h" // Necesario para configurar pines

typedef struct
{
  uint8_t spcr; // Control Register
  uint8_t spsr; // Status Register
  uint8_t spdr; // Data Register
} volatile spi_t;

// Puntero a la dirección base de los registros SPI (0x4C)
volatile spi_t *spi = (spi_t *) 0x4C;


void spi_init(void) {
    // Configurar Pines usando gpio.c
    // Salidas: MOSI, SCK, SS
    gpio_output(SPI_MOSI_PIN);
    gpio_output(SPI_SCK_PIN);
    gpio_output(SPI_SS_PIN);
    
    // Entrada: MISO
    gpio_input(SPI_MISO_PIN);
    // Habilitar Pull-up en MISO (gpio_pin con op=1 ON en modo entrada activa pullup)
    gpio_pin(SPI_MISO_PIN, 1);

    // SS (Pin 10) debe estar ALTO en modo Maestro para evitar 
    // caer en modo esclavo accidentalmente.
    gpio_pin(SPI_SS_PIN, 1); // Set HIGH
    
    // Configurar el Registro de Control SPI (SPCR) usando la struct
    // - Habilitar SPI (SPE = 1)
    // - Configurar como Maestro (MSTR = 1)
    // - Modo SPI 0 (CPOL = 0, CPHA = 0)
    spi->spcr = (1 << SPE) | (1 << MSTR);

    // Establecer velocidad inicial (< 400kHz para SD init)
    spi_set_speed(SPI_CLOCK_DIV_128);
}

void spi_set_speed(spi_clock_div_t clock_div) {
    // Limpiamos los bits de velocidad primero
    spi->spcr &= ~((1 << SPR1) | (1 << SPR0));
    spi->spsr &= ~(1 << SPI2X);

    switch (clock_div) {
        case SPI_CLOCK_DIV_2:
            spi->spsr |= (1 << SPI2X);
            break;
        case SPI_CLOCK_DIV_8:
            spi->spsr |= (1 << SPI2X);
            spi->spcr |= (1 << SPR0);
            break;
        case SPI_CLOCK_DIV_32:
            spi->spsr |= (1 << SPI2X);
            spi->spcr |= (1 << SPR1);
            break;
        case SPI_CLOCK_DIV_64:
            spi->spcr |= (1 << SPR1);
            break;
        case SPI_CLOCK_DIV_128:
            spi->spcr |= (1 << SPR1) | (1 << SPR0);
            break;
        case SPI_CLOCK_DIV_16:
            spi->spcr |= (1 << SPR0);
            break;
        case SPI_CLOCK_DIV_4:
        default:
            break;
    }
}

unsigned char spi_transfer(unsigned char data) {
    // Cargar el byte en el registro de datos (SPDR)
    spi->spdr = data;
    
    // Esperar a que la transferencia se complete (Bit SPIF en SPSR)
    while (!(spi->spsr & (1 << SPIF)));
    
    // Leer el resultado (SPDR)
    return spi->spdr;
}