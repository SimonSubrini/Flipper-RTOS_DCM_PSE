/*
 * main.c - Slave: Audio (SD/DAC) + Matriz LED + Serial
 */

#include <xinu.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "sd_card.h"
#include "dac_mcp4725.h"
#include "serial.h"
#include "led_matrix.h"
#include "timer1.h"

/* --- CONFIGURACIÓN DE AUDIO --- */
#define MUSIC_START_BLOCK   100UL
#define MUSIC_FILE_SIZE     248000UL
#define TOTAL_MUSIC_BLOCKS  (MUSIC_FILE_SIZE / 512)
#define BUFFER_SIZE 400
#define HALF_BUFFER (BUFFER_SIZE / 2) 

/* --- SECUENCIAS LED (EN FLASH/PROGMEM) --- */
// Al usar PROGMEM, estas constantes NO ocupan RAM.
const uint16_t SEQ_U[] PROGMEM = { 0x0000, 0xFFFF}; // Parpadeo
const uint16_t SEQ_V[] PROGMEM = { 0xF000, 0x0F00, 0x00F0, 0x000F, 0x00F0, 0x0F00 }; // Oscila vertical
const uint16_t SEQ_W[] PROGMEM = { 0x8888, 0x4444, 0x2222, 0x1111, 0x2222, 0x4444 }; // Oscila horizontal
const uint16_t SEQ_X[] PROGMEM = { 0xAAAA, 0x5555 }; // Patron ajedrez
const uint16_t SEQ_Y[] PROGMEM = { 0x0000, 0xF000, 0xFF00, 0xFFF0, 0xFFFF, 0x0FFF, 0x00FF, 0x000F }; // Acumulativo horizontal
const uint16_t SEQ_Z[] PROGMEM = { 0x0660, 0x0FF0, 0xFFFF, 0x0FF0 }; // Expansion desde el centro

// Longitudes
#define LEN_U 2
#define LEN_V 6
#define LEN_W 6
#define LEN_X 2
#define LEN_Y 8
#define LEN_Z 4

// Velocidades
#define VEL_1 20
#define VEL_2 10
#define VEL_3 5

/* --- GLOBALES COMPARTIDAS --- */
unsigned char audio_buffer[BUFFER_SIZE]; // 512 bytes RAM

// Semáforo
sid32 sem_sd_request;

// Estado Audio
volatile unsigned int play_index = 0;
volatile uint8_t fill_request_part = 0;
volatile uint8_t is_playing = 0;

// Estado Matriz LED (Volatile para acceso concurrente)

volatile uint16_t *current_seq_ptr = SEQ_U;  // Puntero a Flash donde inicia la secuencia actual
volatile uint8_t current_seq_len = LEN_U;
volatile uint8_t led_speed_cycles = VEL_1; 

/* --- ISR TIMER1 --- */
void audio_isr_logic(void) {
	tx2dac(audio_buffer[play_index]);
	play_index++;

	if (play_index == HALF_BUFFER) {
		fill_request_part = 0;
		signal(sem_sd_request);
	}
	else if (play_index == BUFFER_SIZE) {
		play_index = 0;
		fill_request_part = 1;
		signal(sem_sd_request);
	}
}

/* --- TAREA 1: CARGADOR SD (Prioridad 20) --- */
void task_sd_loader(void) {
    uint16_t current_block = 0;
    while(1) {
        wait(sem_sd_request);
        if (fill_request_part == 0) {
            sd_read_partial(MUSIC_START_BLOCK + current_block, audio_buffer, 0, HALF_BUFFER);
        } else {
            sd_read_partial(MUSIC_START_BLOCK + current_block, audio_buffer, HALF_BUFFER, HALF_BUFFER);
            current_block++;
            if (current_block >= (uint16_t)TOTAL_MUSIC_BLOCKS) current_block = 0;
        }
    }
}

/* --- TAREA 2: MATRIZ LED (Prioridad 15) --- */
void task_led_matrix(void) {
    uint8_t frame_idx = 0;
    uint8_t refresh_counter = 0;
    uint16_t frame_data;

    while(1) {
        frame_data = pgm_read_word(&current_seq_ptr[frame_idx]);
        matrix_render_frame(frame_data);
        refresh_counter++;
        if (refresh_counter >= led_speed_cycles) {
            refresh_counter = 0;
            frame_idx++;
            if (frame_idx >= current_seq_len) {
                frame_idx = 0;
            }
        }
    }
}

/* --- TAREA 3: GESTOR SERIAL (Prioridad 10) --- */
void task_serial(void) {
    char cmd;
    serial_put_str_flash(PSTR("A"));

    while(1) {
        cmd = serial_get_char(); // Bloqueante

        switch(cmd) {
            // --- AUDIO ---
            case 'B': // Play
                if (!is_playing) {
                    timer1_start();
                    is_playing = 1;
                    //serial_put_str_flash(PSTR("Play\r\n"));
                }
                break;
            case 'C': // Pause
                if (is_playing) {
                    timer1_stop();
                    is_playing = 0;
                    tx2dac(0x80);
                    //serial_put_str_flash(PSTR("Pause\r\n"));
                }
                break;

            // --- PATRONES LED ---
            case 'U': 
				current_seq_ptr = SEQ_U; 
				current_seq_len = LEN_U;
				//serial_put_str_flash(PSTR("Matrix-U\r\n"));
				break;
            case 'V': 
				current_seq_ptr = SEQ_V; 
				current_seq_len = LEN_V;
				//serial_put_str_flash(PSTR("Matrix-V\r\n"));
				break;
            case 'W': 
				current_seq_ptr = SEQ_W; 
				current_seq_len = LEN_W;
				//serial_put_str_flash(PSTR("Matrix-W\r\n"));
				break;
            case 'X': 
				current_seq_ptr = SEQ_X; 
				current_seq_len = LEN_X;
				//serial_put_str_flash(PSTR("Matrix-X\r\n"));
				break;
            case 'Y': 
				current_seq_ptr = SEQ_Y; 
				current_seq_len = LEN_Y;
				//serial_put_str_flash(PSTR("Matrix-Y\r\n"));
				break;
            case 'Z': 
				current_seq_ptr = SEQ_Z; 
				current_seq_len = LEN_Z;
				//serial_put_str_flash(PSTR("Matrix-Z\r\n"));
				break;

            // --- VELOCIDAD LED ---
            // Menos ciclos = animación más rápida
            case '1': 
				led_speed_cycles = VEL_1;
				//serial_put_str_flash(PSTR("Matrix-1\r\n"));
				break; // Muy rápido
            case '2': 
				led_speed_cycles = VEL_2;
				//serial_put_str_flash(PSTR("Matrix-2\r\n"));
				break; // Normal
            case '3': 
				led_speed_cycles = VEL_3; 
				//serial_put_str_flash(PSTR("Matrix-3\r\n"));
				break; // Lento
        }
    }
}

/* --- HARDWARE INIT --- */
void hardware_init(void) {
    matrix_init(); 
    
    if (sd_init() != SD_OK) {
        //serial_put_str_flash(PSTR("E-SD\r\n"));
        while(1); 
    }
    dac_init();
	timer1_init(audio_isr_logic);
}

/* --- MAIN --- */
void main(void) {
    hardware_init();
    sleepms(5000); 

    sem_sd_request = semcreate(0);

    // Precarga Audio
    sd_read_partial(MUSIC_START_BLOCK, audio_buffer, 0, HALF_BUFFER);
    sd_read_partial(MUSIC_START_BLOCK, audio_buffer, HALF_BUFFER, HALF_BUFFER);
    
    pid32 pid_audio  = create(task_sd_loader,  180, 20, "sd", 0);
    pid32 pid_matrix = create(task_led_matrix, 120, 15, "led", 0);
    pid32 pid_serial = create(task_serial,     100, 10, "ser", 0);

	if (pid_audio == SYSERR) {
		//serial_put_str_flash(PSTR("Err:RAM Audio\r\n"));
		} else {
		resume(pid_audio);
		//serial_put_str_flash(PSTR("Audio OK\r\n"));
}

	if (pid_matrix == SYSERR) {
		//serial_put_str_flash(PSTR("Err:RAM Matrix\r\n"));
		} else {
		resume(pid_matrix);
		//serial_put_str_flash(PSTR("Matrix OK\r\n"));
	}

	if (pid_serial == SYSERR) {
		//serial_put_str_flash(PSTR("Err:RAM Serial\r\n"));
		} else {
		resume(pid_serial);
		//serial_put_str_flash(PSTR("Serial OK\r\n"));
	}

    return;
}