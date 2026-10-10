#include <stdio.h>

#include "console.h"

#include "usart.h"
#include "cmsis_os2.h"

static uint8_t console_rx_buffer[UART_RX_BUFFER_SIZE];
static uint8_t console_rx_char;

volatile struct uart_buf console_rx;

/*
 * Skip already present data
 */
int uart_buf_empty_nolock(volatile struct uart_buf *state)
{
	state->count = 0;
	state->free = state->size;
	state->underflow = 0;
	state->overflow = 0;
	state->empty = 1;
	state->full = 0;
	state->idx_read = state->idx_write;
	return 0;
}

int uart_buf_empty(volatile struct uart_buf *state)
{
	taskENTER_CRITICAL();
	uart_buf_empty_nolock(state);
	taskEXIT_CRITICAL();
	return 0;
}

int uart_buf_reset_nolock(volatile struct uart_buf *state, int size)
{
	state->size = size;
	uart_buf_empty_nolock(state);
	return 0;
}

int uart_buf_reset(volatile struct uart_buf *state, int size)
{
	taskENTER_CRITICAL();
	uart_buf_reset_nolock(state, size);
	taskEXIT_CRITICAL();
	return 0;
}

/* in case of overflow, erase the buffer */
int uart_buf_write_nolock(volatile struct uart_buf *state, int d, int wr_pos)
{
	if (state->free < d) {
		state->count += state->free;
		state->free = 0;
		state->overflow += 1;
		state->empty = 0;
		state->full = 1;
		state->idx_read = wr_pos;
		state->idx_write = state->idx_read;
		uart_buf_empty_nolock(state);
		return 1;
	} else {
		state->idx_write = (state->idx_write + d) % state->size;
		state->empty = 0;
		state->full = 0;
		state->free -= d;
		state->count += d;
		return 0;
	}
}

int uart_buf_write_from_isr(volatile struct uart_buf *state, int d, int wr_pos)
{
	UBaseType_t ctx = taskENTER_CRITICAL_FROM_ISR();
	int status = uart_buf_write_nolock(state, d, wr_pos);
	taskEXIT_CRITICAL_FROM_ISR(ctx);
	return status;
}

/*
 * Move  buffer idx to perform a read
 */
int uart_buf_read_nolock(volatile struct uart_buf *state, int d)
{
	if (state->count < d) {
		state->free += state->count;
		state->count = 0;
		state->underflow += 1;
		state->empty = 1;
		state->full = 0;
		state->idx_read = state->idx_write;
		return 1;
	} else {
		state->idx_read = (state->idx_read + d) % state->size;
		state->empty = state->idx_read == state->idx_write;
		state->full = 0;
		state->free += d;
		state->count -= d;
		return 0;
	}
}

int uart_buf_read(volatile struct uart_buf *state, int d)
{
	taskENTER_CRITICAL();
	int status = uart_buf_read_nolock(state, d);
	taskEXIT_CRITICAL();
	return status;
}

/*
 * Return a byte if available, else returns -1
 */
int console_get_key(void)
{
	if (console_rx.empty)
		return -1;
	int data = console_rx_buffer[console_rx.idx_read];
	uart_buf_read(&console_rx, 1);
	return data;
}

/*
 * Wait for a byte, eventually loop over the next sys tick
 */
int __io_getchar(void)
{
	while (console_rx.empty)
		osDelay(1);
	int data = console_rx_buffer[console_rx.idx_read];
	uart_buf_read(&console_rx, 1);
	return data;
}

/*
 * Scan a text line smaller than max_size, without timeout
 *  the line ends with \n or \r, these are removed
 *  If the line overflows, cut the line at max_size-1 byte
 *  The last byte is set to 0 (end of text)
 */
int console_scan_line(char *line, int max_size)
{
	for (int i = 0; i < max_size; i++) {
		int data = __io_getchar();
		line[i] = data;
		if (data == '\n' || data == '\r') {
			line[i] = 0;
			return 0;
		}
	}
	line[max_size - 1] = 0;
	return 1;
}
/*
 * Remove anything remaining in the buffer
 */
int console_rx_flush(void)
{
	uart_buf_empty(&console_rx);
	return 0;
}

/* printf goes out on USART1 (ST-LINK virtual COM port), blocking */
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart != &huart1)
		return;
	console_rx_buffer[console_rx.idx_write] = console_rx_char;
	uart_buf_write_from_isr(&console_rx, 1, console_rx.idx_write);
	HAL_UART_Receive_IT(&huart1, &console_rx_char, 1);
}

/* An error (overrun, noise...) aborts reception: restart it */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart1)
		HAL_UART_Receive_IT(&huart1, &console_rx_char, 1);
}

int console_init(void)
{
	uart_buf_reset(&console_rx, UART_RX_BUFFER_SIZE);
	setvbuf(stdin, NULL, _IONBF, 0);
	return HAL_UART_Receive_IT(&huart1, &console_rx_char, 1);
}
