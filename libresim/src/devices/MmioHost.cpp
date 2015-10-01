
#include "MmioHost.hpp"
#include "Log.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <unistd.h>
#include <cassert>
#include <cstdlib>

const uintptr_t GPFSEL1 = 0x200004;
const uintptr_t GPSET0 = 0x20001C;
const uintptr_t GPCLR0 = 0x200028;
const uintptr_t GPPUD = 0x200094;
const uintptr_t GPPUDCLK0 = 0x200098;

const uintptr_t AUX_ENABLES = 0x215004;
const uintptr_t AUX_MU_IO_REG = 0x215040;
const uintptr_t AUX_MU_IER_REG = 0x215044;
const uintptr_t AUX_MU_IIR_REG = 0x215048;
const uintptr_t AUX_MU_LCR_REG = 0x21504C;
const uintptr_t AUX_MU_MCR_REG = 0x215050;
const uintptr_t AUX_MU_LSR_REG = 0x215054;
const uintptr_t AUX_MU_MSR_REG = 0x215058;
const uintptr_t AUX_MU_SCRATCH = 0x21505C;
const uintptr_t AUX_MU_CNTL_REG = 0x215060;
const uintptr_t AUX_MU_STAT_REG = 0x215064;
const uintptr_t AUX_MU_BAUD_REG = 0x215068;

const uintptr_t MMIO_BASE = 0x7e000000;

MmioHost::MmioHost(Log *log) {
	for (int i = 0; i < 5; i++)
	      gpio.gpfsel[i] = 0;
	gpio.gppud = 0;
	gpio.gppudclk0 = 0;

	aux.enables = 0;
	aux.mini_uart.baud_reg = 0;
	
	this->log = log;
}

MmioHost::~MmioHost() {
}

void MmioHost::executeWrite(uintptr_t address,
                             uint64_t value,
                             unsigned int size) {
	if (size != 4) {
		throw std::runtime_error("Unimplemented memory write operand size.");
	}
	uintptr_t offset = address - MMIO_BASE;
	switch (offset) {
		case GPFSEL1:
			gpio.gpfsel[1] = value;
			break;
		case GPPUD:
			gpio.gppud = value;
			break;
		case GPPUDCLK0:
			gpio.gppudclk0 = value;
			break;
		case AUX_ENABLES:
			aux.enables = value;
			break;
		case AUX_MU_IER_REG:
		case AUX_MU_IIR_REG:
		case AUX_MU_LCR_REG:
		case AUX_MU_MCR_REG:
		case AUX_MU_LSR_REG:
		case AUX_MU_MSR_REG:
		case AUX_MU_SCRATCH:
		case AUX_MU_CNTL_REG:
		case AUX_MU_STAT_REG:
		case AUX_MU_BAUD_REG:
			break;

		case AUX_MU_IO_REG:
			/* Alt function 5 for GPIO #14.  */
			if (((gpio.gpfsel[1] >> 12) & 7) == 2
			    && (aux.enables & 1) != 0)
				putchar (value);
			break;
		
		default:
			log->debug ("mmio", "Unknown MMIO address %08x",
				    (int) offset);
	}
}

uint64_t MmioHost::executeRead(uintptr_t address,
                                unsigned int size) {
	if (size != 4) {
		throw std::runtime_error("Unimplemented memory read operand size.");
	}
	uintptr_t offset = address - MMIO_BASE;
	uint64_t value = 0;

	switch (offset) {
		case GPFSEL1:
			value = gpio.gpfsel[1];
			break;
		case GPPUD:
			value = gpio.gppud;
			break;
		case GPPUDCLK0:
			value = gpio.gppudclk0;
			break;
		case AUX_ENABLES:
			value = aux.enables;
			break;
		case AUX_MU_IER_REG:
		case AUX_MU_IIR_REG:
		case AUX_MU_LCR_REG:
		case AUX_MU_MCR_REG:
		case AUX_MU_MSR_REG:
		case AUX_MU_SCRATCH:
		case AUX_MU_CNTL_REG:
		case AUX_MU_STAT_REG:
		case AUX_MU_BAUD_REG:
			break;
		case AUX_MU_LSR_REG:
			/* Transmitter is always empty.  */
			value = 0x20;
			break;
		case AUX_MU_IO_REG:
			break;
		default:
			log->debug ("mmio", "Unknown MMIO address %08x",
				    (int) offset);
	}
	return value;
}
