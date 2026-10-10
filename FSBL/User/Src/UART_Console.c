/*
 * UART_Console.c
 *
 *  Created on: 31 août 2026
 *      Author: mancinis
 */
#include <stdio.h>

#include "UART_Console.h"

#include "stm32n6xx_nucleo.h"
#include "cmsis_os2.h"

extern UART_HandleTypeDef hcom_uart[COMn];

UART_HandleTypeDef *hcom_UART;

#define UART_RX_POLL 0

uint8_t UART_Rx_buffer_DMA[UART_RX_BUFFER_SIZE] __NON_CACHEABLE;

volatile UART_Buffer_State UART_state;

/* DMA last wrote data, used to compute size of new incoming data */

/*
 * Skip already present data
 */
int UART_State_Empty_base(volatile UART_Buffer_State *state)
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

int UART_State_Empty(volatile UART_Buffer_State *state)
{
	taskENTER_CRITICAL();
	UART_State_Empty_base(state);
	taskEXIT_CRITICAL();
	return 0;
}

int UART_State_Reset_base(volatile UART_Buffer_State *state, int size)
{
	state->size = size;
	UART_State_Empty_base(state);
	return 0;
}

int UART_State_Reset(volatile UART_Buffer_State *state, int size)
{
	taskENTER_CRITICAL();
	UART_State_Reset_base(state, size);
	taskEXIT_CRITICAL();
	return 0;
}

/* in case of overflow, erase the buffer */
int UART_State_Write_base(volatile UART_Buffer_State *state, int d, int wr_pos)
{
	if (state->free < d) {
		state->count += state->free;
		state->free = 0;
		state->overflow += 1;
		state->empty = 0;
		state->full = 1;
		state->idx_read = wr_pos;
		state->idx_write = state->idx_read;
		UART_State_Empty_base(state);
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

int UART_State_Write(volatile UART_Buffer_State *state, int d, int wr_pos)
{
	taskENTER_CRITICAL();
	int status = UART_State_Write_base(state, d, wr_pos);
	taskEXIT_CRITICAL();
	return status;
}

int UART_State_Write_From_ISR(volatile UART_Buffer_State *state, int d,
			      int wr_pos)
{
	UBaseType_t ctx = taskENTER_CRITICAL_FROM_ISR();
	int status = UART_State_Write_base(state, d, wr_pos);
	taskEXIT_CRITICAL_FROM_ISR(ctx);
	return status;
}

/*
 * Move  buffer idx to perform a read
 */
int UART_State_Read_base(volatile UART_Buffer_State *state, int d)
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

int UART_State_Read(volatile UART_Buffer_State *state, int d)
{
	taskENTER_CRITICAL();
	int status = UART_State_Read_base(state, d);
	taskEXIT_CRITICAL();
	return status;
}

/*
 * Return a byte if available, else returns -1
 */
uint8_t received_char;
int UART_Get_Key(void)
{
	if (UART_state.empty)
		return -1;
	int data = UART_Rx_buffer_DMA[UART_state.idx_read];
	UART_State_Read(&UART_state, 1);
	return data;
}

/*
 * Wait for a byte, eventually loop over the next sys tick
 */
int __io_getchar(void)
{
#if UART_RX_POLL
	while (UART_state.empty)
		;
#else
	while (UART_state.empty)
		osDelay(1);
#endif
	int data = UART_Rx_buffer_DMA[UART_state.idx_read];
	UART_State_Read(&UART_state, 1);
	return data;
}

/*
 * Scan a text line smaller than max_size, without timeout
 *  the line ends with \n or \r, these are removed
 *  If the line overflows, cut the line at max_size-1 byte
 *  The last byte is set to 0 (end of text)
 */
int UART_Scan_Line(char *line, int max_size)
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
 * Get a buffer of exactly n bytes, without timeout
 */
int UART_Get_Buffer_n(char *buffer, int n)
{
	for (int i = 0; i < n; i++) {
		int data = __io_getchar();
		buffer[i] = data;
	}
	return 0;
}

int UART_Rx_count(void)
{
	return UART_state.count;
}

/*
 * Remove anything remaining in the buffer
 */
int UART_Rx_Flush(void)
{
	UART_State_Empty(&UART_state);
	return 0;
}
extern int UART_DMA_pos_current;
/*
 *
 */

int UART_DMA_Init(int com)
{
	MPU_Config();
	hcom_UART = &hcom_uart[com];
	UART_DMA_pos_current = 0;
	UART_State_Reset(&UART_state, UART_RX_BUFFER_SIZE);
	MX_GPDMA1_Init();
	MX_USART1_UART_Init();
	UART_Start_Reception_DMA();
	setvbuf(stdin, NULL, _IONBF, 0);
	UART_State_Empty(&UART_state);

	return 0;
}
