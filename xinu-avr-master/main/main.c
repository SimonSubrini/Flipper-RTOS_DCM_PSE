/*
 * main.c - Master Pinball 
 */

#include <xinu.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

// Drivers HAL
#include "gpio.h"
#include "serial.h"   
#include "lcd.h"
#include "servo.h"

/* --- CONFIGURACIÓN DE PINES --- */
#define PIN_SCORE_INT  2  // INT0 (PD2) - Sensor de puntos
#define PIN_PHOTOINTERRUPT      3  // PD3 - Sensor de pérdida de vida
#define PIN_RESET_BTN  8  // PB0 - Botón Reset

/* --- CONSTANTES DEL JUEGO --- */
#define MAX_LIVES      3
#define LEVEL_1_SCORE  3000
#define LEVEL_2_SCORE  6000

/* --- ESTADOS DEL JUEGO --- */
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAME_OVER
} game_state_t;


/* --- Códigos de Animación --- */
#define ANIM_NONE       0
#define ANIM_START_GAME 1
#define ANIM_LEVEL_1    2
#define ANIM_LEVEL_2    3
#define ANIM_LEVEL_3    4
#define ANIM_LIFE_LOST  5
#define ANIM_GAME_OVER  6


/* --- GLOBALES COMPARTIDAS --- */
// Volatile porque se modifican en interrupciones o entre tareas
volatile uint8_t anim_request = ANIM_NONE;
volatile uint8_t current_anim = ANIM_NONE;
volatile uint8_t last_anim = ANIM_NONE;
volatile game_state_t current_state = STATE_MENU;
volatile uint16_t score = 0;
volatile uint8_t lives = MAX_LIVES;
volatile uint8_t current_level = 1;
volatile uint8_t update_display_flag = 0; // Semáforo ligero para LCD


/* --- INTERRUPCIÓN: PUNTAJE (INT0) --- */
// Se dispara cuando la bola golpea los postes
void score_isr_logic(void){
    if (current_state == STATE_PLAYING) {
        score += 1; // Sumar puntos
        if (current_level == 1 && score >= LEVEL_1_SCORE) current_level = 2;
        else if (current_level == 2 && score >= LEVEL_2_SCORE) current_level = 3;
        
        update_display_flag = 1; // Avisar a tarea LCD
    }
}

/* --- TAREA 1: LÓGICA DE JUEGO --- */
void task_game_logic(void) {
    uint8_t last_level = 1;

    while(1) {
        switch(current_state) {
            case STATE_MENU:
                // Esperar botón de inicio (Reset)
                // botón con pull-up interno (LOW = presionado)
                anim_request = ANIM_START_GAME; 
                if (gpio_pin(PIN_RESET_BTN, 3) == 0) { // 3 = GET
                    // Reiniciar variables
                    score = 0;
                    lives = MAX_LIVES;
                    current_level = 1;
                    last_level = 1;
                    current_state = STATE_PLAYING;
                    update_display_flag = 1;
                    sleepms(100); 
					anim_request = ANIM_LEVEL_1; // Arranca el bucle de servos nivel 1
                }
                break;

            case STATE_PLAYING:                
                // Detección de flanco (si el sensor manda pulso positivo)
                if (gpio_pin(PIN_PHOTOINTERRUPT, 3) == 1) { // Comando 3 de gpio_pin -> lectura
	                update_display_flag = 1;
                    if (lives > 1) {
						lives--;
						last_anim = current_anim;
						anim_request = ANIM_LIFE_LOST;
					}

                    else{
                        current_state = STATE_GAME_OVER;
                        anim_request = ANIM_GAME_OVER;
                    }
                    sleepms(3000); 
                }

                // Lógica de Niveles 
                if (current_level != last_level) {
	                if (current_level == 2) {
		                lives++;
		                anim_request = ANIM_LEVEL_2; 
		            } else if (current_level == 3) {
		                lives++;
		                anim_request = ANIM_LEVEL_3;
	                }
	                // Caso especial: Si se resetea el juego y volvemos a nivel 1
	                else if (current_level == 1) {
		                anim_request = ANIM_LEVEL_1;
	                }
	                
	                last_level = current_level;
	                update_display_flag = 1;
                }
                break;

            case STATE_GAME_OVER:
                // Esperar botón reset para volver al menu
                if (gpio_pin(PIN_RESET_BTN, 3) == 0) {
                     current_state = STATE_MENU;
                     update_display_flag = 1;
                     sleepms(500);
                }
                break;
        }
        
        // Ceder CPU
        sleepms(20);
    }
}

/* --- TAREA 2: GESTOR LCD  --- */
void task_lcd_display(void) {
    char buffer[17]; // Buffer para linea LCD (16 chars + null)
    uint8_t last_known_state = 255; // Forzar update inicial

    lcd_clear();

    while(1) {
        // Solo actualizamos si se levantó la bandera o cambió el estado
        if (update_display_flag || current_state != last_known_state) {
            
            lcd_clear(); 

            switch(current_state) {
                case STATE_MENU:
					lcd_set_cursor(0, 0);
					lcd_print_flash(PSTR("    PINBALL     "));
					lcd_set_cursor(1, 0);
                    lcd_print_flash(PSTR("  PRESS START   "));
                    break;

                case STATE_PLAYING:
                    
                    // Linea 1: Vidas
					lcd_set_cursor(0, 0);
					lcd_print_flash(PSTR("LIVES: "));
					lcd_print_uint16(lives); 
					lcd_print_flash(PSTR("  LVL: ")); 
					lcd_print_uint16(current_level);
                    // Linea 2: Score
					lcd_set_cursor(1, 0);
					lcd_print_flash(PSTR("SCORE: "));
                    lcd_print_uint16(score);
                    break;

                case STATE_GAME_OVER:
					lcd_set_cursor(0, 0);
					lcd_print_flash(PSTR("   GAME OVER   "));
					lcd_set_cursor(1, 0);
					lcd_print_flash(PSTR("SCORE: "));
					lcd_print_uint16(score);
                    break;
            }

            update_display_flag = 0;
            last_known_state = current_state;
        }
        sleepms(100);
    }
}

/* --- TAREA DE ANIMACIÓN --- */
void task_animator(void) {
	current_anim = ANIM_NONE;
	last_anim = 255; // Valor imposible para forzar ejecución inicial

	while(1) {
		// Verificar si hay nueva solicitud desde la lógica del juego
		if (anim_request != ANIM_NONE) {
			current_anim = anim_request;
			anim_request = ANIM_NONE; // Consumo la solicitud
		}

		// Ejecutar lógica según el estado actual
		switch(current_anim) {
			
			case ANIM_NONE:
				// Estado de reposo
				break;

			case ANIM_START_GAME:
				// Solo se ejecuta una vez al entrar
				if (current_anim != last_anim) {
					servo_set_gate(1, 0);
					servo_set_gate(2, 0);
					serial_put_str_flash(PSTR("BU1")); //Play a la musica, parpadeo lento
				}
				break;

			case ANIM_LEVEL_1:
				if (current_anim != last_anim) {
					serial_put_str_flash(PSTR("Y1")); //acumulativo lento
				}
				last_anim = current_anim; // Como abajo tengo un delay grande el estado de animacion puede cambiar y yo notarlo solo al final del bucle
			
				// FASE BUCLE: Movimiento de Servos (Obstáculos)
				// Secuencia: Abrir A -> Esperar -> Cerrar A/Abrir B -> Esperar -> Cerrar B
				servo_set_gate(1, 1); // Abrir A
				servo_set_gate(2, 0);
				sleep(4); 
				if (current_anim != last_anim) {
					break; //como la animación es "lenta" hago esta comprobación a mitad de bucle
				}
			
				servo_set_gate(1, 0); // Cerrar A
				servo_set_gate(2, 1); // Abrir B
				sleep(4);
				break;

			case ANIM_LEVEL_2:
				serial_put_str_flash(PSTR("V2")); //osc. vertical medio
				servo_set_gate(1, 1); // Abrir A
				servo_set_gate(2, 0);
				last_anim = current_anim; // Como abajo tengo un delay grande el estado de animacion puede cambiar y yo notarlo solo al final del bucle
				
				sleep(3);          
				if (current_anim != last_anim) {
					break; //como la animación es "lenta" hago esta comprobación a mitad de bucle
				}
				
				serial_put_str_flash(PSTR("W2")); //osc. horizontal medio
				servo_set_gate(1, 0); // Cerrar A
				servo_set_gate(2, 1); // Abrir B
				sleep(3);
				break;

			case ANIM_LEVEL_3:
				serial_put_str_flash(PSTR("Z3")); //expansion rapido
				servo_set_gate(1, 1); // Abrir A
				servo_set_gate(2, 0);
				last_anim = current_anim; // Como abajo tengo un delay grande el estado de animacion puede cambiar y yo notarlo solo al final del bucle
				
				sleep(2);           
				if (current_anim != last_anim) {
					break; //como la animación es "lenta" hago esta comprobación a mitad de bucle
				} 
				
				serial_put_str_flash(PSTR("U3")); //Parpadeo rapido
				servo_set_gate(1, 0); // Cerrar A
				servo_set_gate(2, 1); // Abrir B
				sleep(2);
				break;

			case ANIM_LIFE_LOST:
				// Este es un evento de una sola vez 
				if (current_anim != last_anim) {
					serial_put_str_flash(PSTR("CX3")); // Silencio, ajedrez rapido
					sleep(4);
					serial_put_str_flash(PSTR("B")); // Play
					current_anim = last_anim;
				}
				break;
			
			case ANIM_GAME_OVER:
				// Este es un evento de una sola vez 
				if (current_anim != last_anim) {
					serial_put_str_flash(PSTR("CX3")); // Silencio, ajedrez rapido
					sleep(4);
					serial_put_str_flash(PSTR("U1")); // parpadeo lento
				}
				sleep(1);
				break;
		}

		// Actualizar estado anterior
		last_anim = current_anim;
		
		sleepms(50);
	}
}
/* --- HARDWARE INIT --- */
void sys_init(void) {
	serial_init();          // Driver Serial (9600)
    // Pines de Entrada
    gpio_input(PIN_PHOTOINTERRUPT);
    gpio_input(PIN_RESET_BTN);
    gpio_input(PIN_SCORE_INT);

    gpio_attach_int0(INT_RISING_EDGE, score_isr_logic);
    
    // Activar Pull-ups
    gpio_pin(PIN_RESET_BTN, 1); // Pull-up ON
    gpio_pin(PIN_SCORE_INT, 1); // Pull-up ON

    servo_init();  // Driver Servos
    lcd_init();
	lcd_set_cursor(0, 0);;
	lcd_print_flash(PSTR("Cargando..."));
	char cmd;
	do { 
		cmd = serial_get_char();
		}
	while(cmd != 'A');  // Bloqueante, espero a que llegue "A", que indica que el slave esta listo
	
}

/* --- MAIN --- */
void main(void) {
    sys_init();
	
	// Habilitar interrupciones globales (necesario para INT0 y Timer1 PWM si usara ISR)
	sei();
	
	pid32 pid_game  = create(task_game_logic,  150, 20, "log", 0);
	pid32 pid_animator = create(task_animator, 150, 15, "ani", 0);
	pid32 pid_lcd = create(task_lcd_display, 200, 10, "lcd", 0);

	if (pid_game == SYSERR) {
		//serial_put_str_flash(PSTR("Err:RAM Game\r\n"));
		} else {
		resume(pid_game);
		//serial_put_str_flash(PSTR("Game OK\r\n"));
	}

	if (pid_animator == SYSERR) {
		//serial_put_str_flash(PSTR("Err:RAM Anim\r\n"));
		} else {
		resume(pid_animator);
		//serial_put_str_flash(PSTR("Anim OK\r\n"));
	}

	if (pid_lcd == SYSERR) {
		//serial_put_str_flash(PSTR("Err:RAM LCD\r\n"));
		} else {
		resume(pid_lcd);
		//serial_put_str_flash(PSTR("LCD OK\r\n"));
	}

    return;
}