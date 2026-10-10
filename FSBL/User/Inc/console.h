/*
 * console.h
 *
 *  Created on: 31 août 2026
 *      Author: mancinis
 */

#pragma once

#include "FreeRTOS.h"
#include "task.h"

#define UART_RX_BUFFER_SIZE 64 //1024

int console_init(void);

int console_get_key(void);
int console_scan_line(char *line, int max_size);

int console_rx_flush(void);

/* internal stuff */

struct uart_buf {
	volatile int size;
	volatile int count;
	volatile int free;
	volatile int idx_read;
	volatile int idx_write;
	volatile int empty;
	volatile int full;
	volatile int overflow;
	volatile int underflow;
};

int uart_buf_write_from_isr(volatile struct uart_buf *state, int d, int wr_pos);
