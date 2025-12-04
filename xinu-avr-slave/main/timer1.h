/*
 * timer1.h
 */
#ifndef TIMER1_H
#define TIMER1_H

// Inicializa el timer y registra la función de callback para la interrupción
void timer1_init(void (*isr_callback)(void));

// Habilita la interrupción (Play)
void timer1_start(void);

// Deshabilita la interrupción (Pause)
void timer1_stop(void);

#endif