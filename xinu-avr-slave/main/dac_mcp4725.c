/*
 * dac_mcp4725.c
 */

#include "dac_mcp4725.h"

// --- Constantes ---

// Dirección I2C de 7 bits del MCP4725.
#define DAC_ADDRESS (0x60)

// Address 0x60 (1100000) << 1 = 0xC0. Bit 0 (W) es 0.
#define DAC_ADDR_WRITE 0xC0

// Byte de comando para "Escribir en el Registro del DAC" (Ver p.18 del datasheet)
// C2=0, C1=1, C0=0. (Modo=Normal, PowerDown=Normal)
// Esto es 0b01000000
#define DAC_CONTROL_WRITE_DAC (0x40)


void dac_init(void) {
    i2c_init();
}

void tx2dac(unsigned char pcm_value) {
    // El protocolo de escritura "Write DAC Register" es:
    // START -> ADDR+W -> ACK -> CMD -> ACK -> DATA_H -> ACK -> DATA_L -> ACK -> STOP

    // 1. Enviar condición START
    i2c_start();
    
    // 2. Enviar dirección del esclavo + bit de Escritura (W)
    i2c_write(DAC_ADDR_WRITE);
    
    // 3. Enviar el byte de comando (0x40)
    i2c_write(DAC_CONTROL_WRITE_DAC);
    
    // 4. Enviar los 8 bits más significativos (MSB)
    i2c_write(pcm_value);
    
    // 5. Enviar los 4 bits menos significativos (LSB)
    i2c_write(0x00);
    
    // 6. Enviar condición STOP
    i2c_stop();
}