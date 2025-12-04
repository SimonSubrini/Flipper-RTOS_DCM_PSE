#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

void servo_init(void);

// id: 1 o 2. open: 1=Abrir, 0=Cerrar
void servo_set_gate(uint8_t id, uint8_t open);

#endif