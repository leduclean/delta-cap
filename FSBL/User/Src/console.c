#include <errno.h>
#include <stdio.h>

#include "cmsis_os2.h"
#include "console.h"
#include "syscalls.h"
#include "usart.h"

#define CONSOLE_RX_SIZE 64 /* must be a power of 2 */

/**
 * @brief Received data form the uart IRQ.
 */
static uint8_t console_rx_char;

/**
 * @brief FIFO CMSIS queue to receive and buffer data of the uart.
 */
static osMessageQueueId_t console_rx;

void console_rx_flush(void)
{
	osMessageQueueReset(console_rx);
}

int console_get_key(void)
{
	uint8_t c;
	int res;

	res = osMessageQueueGet(console_rx, &c, NULL, osWaitForever);
	if (res < 0) {
		return res;
	}

	return c;
}

int _write([[maybe_unused]] int file, char *ptr, int len)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
	return len;
}

ssize_t console_scan_line(char *line, size_t max_size)
{
	size_t i = 0;

	if (!max_size)
		return -EINVAL;

	for (;;) {
		int c = console_get_key();
		if (c == '\n' || c == '\r') {
			printf("\r\n");
			break;
		}

		if (c == '\b' || c == 0x7f) {
			if (i > 0) {
				i--;
				printf("\b \b");
			}
			continue;
		}

		if (i < max_size - 1) {
			line[i++] = c;
			putchar(c);
		}
	}

	line[i] = '\0';
	return i;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart != &huart1)
		return;

	osMessageQueuePut(console_rx, &console_rx_char, 0, 0);
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
	console_rx = osMessageQueueNew(CONSOLE_RX_SIZE, sizeof(uint8_t), NULL);
	if (!console_rx)
		return -EINVAL;

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	return HAL_UART_Receive_IT(&huart1, &console_rx_char, 1);
}
