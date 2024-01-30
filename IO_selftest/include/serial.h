/*
 * 
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_



#ifdef __cplusplus
extern "C" {
#endif

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 12
#define UART_RX_PIN 13
#define UART_CTS_PIN 14
#define UART_RTS_PIN 15

void setup_uart(void);

#ifdef __cplusplus
}
#endif

#endif //

