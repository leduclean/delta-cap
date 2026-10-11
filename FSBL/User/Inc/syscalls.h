/**
 * @file
 * @brief newlib syscall hooks overriding the weak stubs of Core/Src/syscalls.c.
 */

#pragma once

/**
 * @brief Backs stdout/stderr: sends the buffer on USART1, blocking.
 *
 * @param file The file descriptor, ignored.
 * @param ptr The bytes to send.
 * @param len The number of bytes.
 * @return len.
 */
int _write(int file, char *ptr, int len);
