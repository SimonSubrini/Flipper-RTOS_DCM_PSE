// Script desarrollado en la cursada de PSE

static inline void delay_ms(volatile int ms) {
	// PARA 16MHZ
	while (ms--) {
		volatile int us = 1000;
		while (us--) {
			asm volatile(
			"nop"
			"\n\t"
			"nop"
			"\n\t");
		}
	}
}