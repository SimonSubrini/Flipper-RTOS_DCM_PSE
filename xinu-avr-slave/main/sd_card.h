/*
 * sd_card.h
 */

#ifndef SD_CARD_H_
#define SD_CARD_H_


// --- Definiciones del Módulo SD ---
typedef enum {
    SD_OK = 0,                 // Operación exitosa
    SD_NOK					   // Fallo en la operación
} SD_Status_t;

SD_Status_t sd_init(void);

SD_Status_t sd_read_partial(unsigned long block_addr, unsigned char* buffer,unsigned int offset, unsigned int count);


#endif /* SD_CARD_H_ */