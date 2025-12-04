/*
 * sd_card.c
 */

#include "sd_card.h"
#include "spi.h"
#include "gpio.h"

// --- Comandos del protocolo SD (modo SPI) ---
#define CMD0   (0x40 | 0)   // GO_IDLE_STATE
#define CMD8   (0x40 | 8)   // SEND_IF_COND
#define CMD17  (0x40 | 17)  // READ_SINGLE_BLOCK
#define CMD24  (0x40 | 24)  // WRITE_BLOCK
#define CMD55  (0x40 | 55)  // APP_CMD
#define CMD58  (0x40 | 58)  // READ_OCR
#define ACMD41 (0x40 | 41)  // (APP_CMD) SEND_OP_COND

#define READ_IDLE_STATE (0x01)
#define READ_READY_STATE (0x00)
#define TOKEN_DATA_START (0xFE)
#define TOKEN_WRITE_ACCEPTED (0x05)

#define SD_TIMEOUT 5000


static unsigned char is_sdhc = 0; // Flag: 1 si es SDHC, 0 si es SDSC

static void sd_select(void) {
	gpio_pin(SPI_SS_PIN, 0); // 0 = LOW = Activo
}

static void sd_deselect(void) {
	gpio_pin(SPI_SS_PIN, 1); // 1 = HIGH = Inactivo
}

static unsigned char sd_read_response(void) {
    unsigned char i = 0;
    unsigned char response;
    
    for (i = 0; i < 8; i++) {
        response = spi_receive();
        if (response != 0xFF) {
            return response; 
        }
    }
    return 0xFF; 
}

static void sd_send_command(unsigned char cmd, unsigned long arg) {
    spi_send(cmd);
    spi_send((unsigned char)(arg >> 24));
    spi_send((unsigned char)(arg >> 16));
    spi_send((unsigned char)(arg >> 8));
    spi_send((unsigned char)(arg));
    
    if (cmd == CMD0) {
        spi_send(0x95);
    } else if (cmd == CMD8) {
        spi_send(0x87);
    } else {
        spi_send(0xFF); 
    }
}


SD_Status_t sd_init(void) {
    unsigned char response, r7_payload[4];
	unsigned int timer;

    spi_init();
    
    
    // Secuencia de "despertar" (mínimo 74 ciclos de reloj con CS en ALTO)
    sd_deselect();
    for (int i = 0; i < 10; i++) {
        spi_send(0xFF); // 10 bytes = 80 ciclos
    }
    
    sd_select();
    
    sd_send_command(CMD0, 0);
    if (sd_read_response() != READ_IDLE_STATE) {
        sd_deselect();
        return SD_NOK;
    }
    
    // Envio CMD8 (SEND_IF_COND) - Para identificar SDv2
    sd_send_command(CMD8, 0x000001AA); 
    if (sd_read_response() == READ_IDLE_STATE) {
        spi_receive(); // Byte 0 reservado
        spi_receive(); // Byte 1 reservado
        spi_receive(); // Byte 2 reservado 
        if (spi_receive() != 0xAA) { // Byte 3 check pattern
	        sd_deselect();
	        return SD_NOK;
        }
    } else {
        sd_deselect();
        return SD_NOK; // CMD8 no soportado (no es SDv2)
    }

    timer = SD_TIMEOUT; 
    do {
        // Envio CMD55 (APP_CMD)
        sd_send_command(CMD55, 0);
        sd_read_response();
        
        // Envio ACMD41 (SEND_OP_COND)
        // El bit HCS (Host Capacity Support) (bit 30) le dice a la tarjeta que soporta SDHC.
        sd_send_command(ACMD41, 0x40000000); 
        response = sd_read_response();
        
        timer--;
        if (timer == 0) {
            sd_deselect();
            return SD_NOK;
        }
    } while (response != READ_READY_STATE); // Repetir hasta que la tarjeta salga de IDLE

    // Leer el registro OCR (CMD58) para ver si es SDHC
    sd_send_command(CMD58, 0);
    if (sd_read_response() == READ_READY_STATE) {
	    unsigned char ocr_first_byte = spi_receive();
	    is_sdhc = (ocr_first_byte & 0x40) ? 1 : 0;
	    
	    // Descartar los otros 3 bytes del OCR
	    spi_receive(); 
		spi_receive(); 
		spi_receive();
    }
    
    sd_deselect();
    spi_set_speed(SPI_CLOCK_DIV_4); // 16MHz / 4 = 4 MHz
    
    return SD_OK;
}

SD_Status_t sd_read_partial(unsigned long block_addr, unsigned char* buffer, unsigned int offset, unsigned int count) {
    unsigned char response;
    unsigned int i;

    // Validación básica para evitar desbordamientos 
    if ((offset + count) > 512) return SD_NOK; 

    if (!is_sdhc) {
        block_addr *= 512;
    }

    sd_select();

    // Envio comando CMD17
    sd_send_command(CMD17, block_addr);
    
    if (sd_read_response() != READ_READY_STATE) {
        sd_deselect();
        return SD_NOK;
    }

    // Esperar token 0xFE
	i = SD_TIMEOUT;
    do {
        response = spi_receive();
        i--;
        if (i == 0) {
            sd_deselect();
            return SD_NOK;
        }
    } while (response != TOKEN_DATA_START);
    
    // Descartar bytes previos (Offset inicial)
    for (i = 0; i < offset; i++) {
        spi_receive(); // Leemos del bus pero no guardamos nada
    }

    //  Leer y guardar los bytes que nos interesan
    // Nota: Uso puntero directo para máxima velocidad
	
	unsigned char *ptr = buffer + offset; // Calcular dirección base UNA vez
	for (i = 0; i < count; i++) {
		*ptr++ = spi_receive(); // Escribir e incrementar puntero 
	}

    // Descarto bytes restantes para completar el protocolo SD (512 bytes total)
    // Calculo cuánto falta para terminar el bloque
    for (i = 0; i < 512 - (offset + count); i++) {
        spi_receive();
    }

    // Descarto CRC (comprobacion de redundancia ciclica) (2 bytes)
    spi_receive();
    spi_receive();
    
    sd_deselect();
    return SD_OK;
}