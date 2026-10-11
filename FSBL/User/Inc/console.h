/**
 * @file
 * @brief Console helpers using USART1 RX irq to read user inputs.
 */

#pragma once

#include <sys/types.h>

/**
 * @brief Start interrupt-driven reception on USART1.
 *
 * Must be called after osKernelInitialize().
 *
 * @return 0 on success, -EINVAL or a HAL status on failure.
 */
int console_init(void);

/**
 * @brief Read a received byte, sleeps until one arrives.
 *
 * @return The byte.
 */
int console_get_key(void);

/**
 * @brief Scan a line of the uart entry, blocking.
 *
 * The line ends on '\n' or '\r' (stripped) and is NUL terminated. Longer
 * lines are cut at max_size - 1 bytes, the rest stays in the rx buffer.
 *
 * @param line[out] The destination line.
 * @param max_size The size of line, including the NUL.
 * @return The length of the line on success, -EINVAL if max_size is 0.
 */
ssize_t console_scan_line(char *line, size_t max_size);

/**
 * @brief Drop every byte pending in the rx buffer.
 */
void console_rx_flush(void);
