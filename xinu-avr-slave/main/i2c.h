/*
 * i2c.h - Driver I2C (TWI)
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

// Inicializa el bus I2C a 400kHz (Fast Mode)
void i2c_init(void);

// Envía condición de Start
void i2c_start(void);

// Envía condición de Stop
void i2c_stop(void);

// Escribe un byte en el bus y espera el ACK
void i2c_write(uint8_t data);


#endif /* I2C_H_ */