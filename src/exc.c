#include "exc.h"
#include "peripherals/mini_uart.h"
enum stat{
	tx_enable=0x2,
	rx_enable=0x4
};

void enable_mini_uart_interrupt(){
	// IER=1 for receive(get) interrupt, 2 for transmit (send) interrupt“
	*IRQs1 |= (1<<29);	
	*AUX_MU_IER_REG = 0x1;
}

/*
 * Implement all system call here.
 */
void svc_entry(unsigned int spsr, unsigned int elr, unsigned esr) {
	if (esr>>26 != 0b010101){
		uart_hex(esr);
		uart_puts("is NOT an aarch64 svc, see https://developer.arm.com/documentation/ddi0601/2020-12/AArch64-Registers/ESR-EL1--Exception-Syndrome-Register--EL1-\n");
		return;
	}
	int call = (esr & 0xFFFFFF);
	switch (call) {
	case 0:
		// spsr, esr's [63:32] are both RES0, use 32 bit show instead
		uart_puts("\n\nspsr_elx:\t");
		uart_hex(spsr);
		uart_puts("\nelr:\t\t");
		uart_hex(elr);
		uart_puts("\nesr\t\t");
		uart_hex(esr);
		break;
	default:
		uart_puts("System call/Behavior not implemented!\n");
		break;
	}
}

void sp_elx_handler(){
	if (*IRQs1 & (1<<29)){
		int uart = *AUX_MU_IIR_REG & 0b110;
		if (uart==tx_enable){
			uart_transmit_handler();
		} else if (uart==rx_enable) {
			uart_receive_handler();
		} else {
			uart_puts("Asynchronous IO anomally!!!");
		}
	}
	else {
		asm volatile(
			"mrs x0, cntfrq_el0;"
			"mrs x1, cntpct_el0;"
			"stp lr, x0, [sp, #-16]!;"
			"bl print_core_timer;"
			"ldp lr, x0, [sp], #16;"
			"lsl x0, x0, #1;"
			"msr cntp_tval_el0, x0;"
			"ret;");
	}
}

/**
 * As the refrence from BCM2835 p.12, AUX_MU_IER_REG:
 * Bit 1 represents the "rx" bit,
 * Bti - represents the "tx" bit.
 * Following 4 operations encapsulated the enable/disable operations.
 */
void disable_tx(){
	*AUX_MU_IER_REG &= 0x1;
}

void disable_rx(){
	*AUX_MU_IER_REG &= 0x2;
}

void enable_tx(){
	*AUX_MU_IER_REG |= 0x1;
}

void exable_rx(){
	*AUX_MU_IER_REG |= 0x2;
}
