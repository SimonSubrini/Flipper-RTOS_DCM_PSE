// Script desarrollado en la cursada de PSE

static inline void delay_us(volatile int us) {
	// PARA 16MHZ

	while (us--) {
		asm volatile(
		"nop"
		"\n\t"
		"nop"
		"\n\t"
		"nop"
		"\n\t"
		"nop"
		"\n\t"
		"nop"
		"\n\t"
		"nop"
		"\n\t"
		"nop"
		"\n\t");
	}
	asm volatile(
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	"nop"
	"\n\t"
	);
	
}