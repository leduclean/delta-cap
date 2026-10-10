/*
 * UART_Console.h
 *
 *  Created on: 31 août 2026
 *      Author: mancinis
 */

#ifndef APPLICATION_USER_NUCLEON6_UART_CONSOLE_UART_CONSOLE_H_
#define APPLICATION_USER_NUCLEON6_UART_CONSOLE_UART_CONSOLE_H_

#include "FreeRTOS.h"
#include "task.h"

#define UART_RX_BUFFER_SIZE 64 //1024

int UART_DMA_Init(int com);

int UART_Get_Key(void);
int UART_Scan_Line(char *line, int max_size);
int UART_Get_Buffer_n(char *buffer, int n);

int UART_Rx_Flush(void);
int UART_Rx_count(void);

/* internal stuff */

typedef struct UART_Buffer_State {
	volatile int size;
	volatile int count;
	volatile int free;
	volatile int idx_read;
	volatile int idx_write;
	volatile int empty;
	volatile int full;
	volatile int overflow;
	volatile int underflow;

} UART_Buffer_State;
int UART_State_Write_From_ISR(volatile UART_Buffer_State *state, int d,
			      int wr_pos);
void UART_Start_Reception_DMA(void);
void MX_GPDMA1_Init(void);
void MX_USART1_UART_Init(void);
void MPU_Config(void);

#endif /* APPLICATION_USER_NUCLEON6_UART_CONSOLE_UART_CONSOLE_H_ */
