#include <errno.h>
#include <stdio.h>

#include "console.h"

#include "usart.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"

#define CONSOLE_RX_SIZE 64 /* must be a power of 2 */

/*
 * Single producer (UART ISR) / single consumer (task) ring buffer,
 * lock-free: the ISR only writes head, the task only writes tail.
 * Indices run freely, head - tail is the number of pending bytes.
 */
static struct {
	uint8_t buf[CONSOLE_RX_SIZE];
	volatile unsigned int head;
	volatile unsigned int tail;
} console_rx;

static uint8_t console_rx_char;

static StaticSemaphore_t console_rx_sem_cb;
static const osSemaphoreAttr_t console_rx_sem_attr = {
	.name = "console_rx",
	.cb_mem = &console_rx_sem_cb,
	.cb_size = sizeof(console_rx_sem_cb),
};

static osSemaphoreId_t console_rx_sem;

/* Pop one byte, -1 if empty */
static int console_rx_pop(void)
{
	unsigned int tail = console_rx.tail;

	if (tail == console_rx.head)
		return -1;

	__DMB(); // Read head before the bit is published
	int c = console_rx.buf[tail & (CONSOLE_RX_SIZE - 1)];
	__DMB(); // Read the byte before advancing the tail
	console_rx.tail = tail + 1;
	return c;
}

/* Called from ISR, drops the byte if the buffer is full */
static void console_rx_push(uint8_t c)
{
	unsigned int head = console_rx.head;

	if (head - console_rx.tail >= CONSOLE_RX_SIZE)
		return;

	console_rx.buf[head & (CONSOLE_RX_SIZE - 1)] = c;
	__DMB(); // Publish byte before moving head
	console_rx.head = head + 1;
	osSemaphoreRelease(console_rx_sem);
}

int console_get_key(void)
{
	int c;
	while ((c = console_rx_pop()) < 0) {
		// Semphore is a wake up hint
		osSemaphoreAcquire(console_rx_sem, osWaitForever);
	}
	return c;
}

/* scanf/getchar read from the console */
int __io_getchar(void)
{
	return console_get_key();
}

/* printf goes out on USART1 (ST-LINK virtual COM port), blocking */
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

ssize_t console_scan_line(char *line, size_t max_size)
{
	size_t i;

	if (!max_size)
		return -EINVAL;

	for (i = 0; i < max_size - 1; i++) {
		int c = console_get_key();

		if (c == '\n' || c == '\r')
			break;
		line[i] = c;
	}

	line[i] = '\0';
	return i;
}

void console_rx_flush(void)
{
	console_rx.tail = console_rx.head;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart != &huart1)
		return;

	console_rx_push(console_rx_char);
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
	console_rx_sem = osSemaphoreNew(1, 0, &console_rx_sem_attr);
	if (!console_rx_sem)
		return -EINVAL;

	setvbuf(stdin, NULL, _IONBF, 0);
	return HAL_UART_Receive_IT(&huart1, &console_rx_char, 1);
}
