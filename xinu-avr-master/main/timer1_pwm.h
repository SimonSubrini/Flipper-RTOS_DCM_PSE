#ifndef TIMER1_PWM_H
#define TIMER1_PWM_H

#include <stdint.h>

// Mapeo de Pines Arduino (PB1=9, PB2=10)
#define SERVO_PIN_A  9
#define SERVO_PIN_B  10

// Inicializa el Timer1 en modo Fast PWM (ICR1 Top)
void timer1_init(void);

// Establece el ciclo de trabajo (duty) en ticks de reloj (0.5us por tick)
// channel: 1 (Servo A), 2 (Servo B)
void timer1_pwm_set_duty(uint8_t channel, uint16_t ticks);

#endif