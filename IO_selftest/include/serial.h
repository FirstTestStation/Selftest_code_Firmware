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


// Default uart configuration
#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE


#define SP9_6K      0     // used on bit field
#define SP38_4K     1     // used on bit field
#define SP57_6K     2     // used on bit field
#define SP115_2K    3     // used on bit field

#define STOP1      1      // 1- stop bit field
#define STOP2      2      // 2- stop bit field

#define UPAR_NONE  0      // Uart parity 
#define UPAR_EVEN  1      // Uart parity 
#define UPAR_ODD   2      // Uart parity

#define D5BIT      0      // define 5 bits uarts (offset of 5 will be added)
#define D6BIT      1      // define 6 bits uarts (offset of 5 will be added)
#define D7BIT      2      // define 7 bits uarts (offset of 5 will be added)
#define D8BIT      3      // define 8 bits uarts (offset of 5 will be added)

#define HAND_NO    0      // define no handshake (RTS/CTS)
#define HAND_YES   1      // define handshake enabled (RTS/CTS)


// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 12
#define UART_RX_PIN 13
#define UART_CTS_PIN 14
#define UART_RTS_PIN 15

void enable_uart(uint8_t rts_cts);
void disable_uart(uint8_t mode);
void set_default_serial(void);
void set_uart_protocol(uint8_t cfg_uart, char *resultstr);
uint8_t get_uart_protocol(char *resultstr);
void test_serial_command(void);

#ifdef __cplusplus
}
#endif

#endif //

