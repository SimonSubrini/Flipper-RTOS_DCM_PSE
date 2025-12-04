#include "servo.h"
#include "timer1_pwm.h"

// Calibración SG90 (Microsegundos)
#define TICKS_OPEN  5000 // 2500us - me extiendo de los limites para forzar a que el servo gire un poco más
#define TICKS_CLOSE 1500 // 1500us

void servo_init(void) {
	timer1_init();
	// Iniciar cerrados
	servo_set_gate(1, 0);
	servo_set_gate(2, 0);
}

void servo_set_gate(uint8_t id, uint8_t open) {
	uint16_t t = open ? TICKS_OPEN : TICKS_CLOSE;
	timer1_pwm_set_duty(id, t);
}