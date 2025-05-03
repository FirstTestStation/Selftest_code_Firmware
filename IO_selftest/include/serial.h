/**
 * @file    serial.h
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Header function to UART serial communication
 *
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 *
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#include <stdint.h>

#ifndef _SERIAL_H_
#    define _SERIAL_H_

#    ifdef __cplusplus
extern "C"
{
#    endif

/**
 * @brief Default UART configuration.
 */
#    define UART_ID uart0           ///< UART instance to be used.
#    define BAUD_RATE 115200        ///< Default UART baud rate.
#    define DATA_BITS 8             ///< Default number of data bits.
#    define STOP_BITS 1             ///< Default number of stop bits.
#    define PARITY UART_PARITY_NONE ///< Default UART parity (none).

/**
 * @brief Baud rate options for bit fields.
 */
#    define SP9_6K 0   ///< 9600 baud rate.
#    define SP38_4K 1  ///< 38400 baud rate.
#    define SP57_6K 2  ///< 57600 baud rate.
#    define SP115_2K 3 ///< 115200 baud rate.

/**
 * @brief Stop bit options for bit fields.
 */
#    define STOP1 1 ///< 1 stop bit.
#    define STOP2 2 ///< 2 stop bits.

/**
 * @brief Parity options for bit fields.
 */
#    define UPAR_NONE 0 ///< No parity.
#    define UPAR_EVEN 1 ///< Even parity.
#    define UPAR_ODD 2  ///< Odd parity.

/**
 * @brief Data bit size options for bit fields (offset of 5 added).
 */
#    define D5BIT 0 ///< 5-bit data size.
#    define D6BIT 1 ///< 6-bit data size.
#    define D7BIT 2 ///< 7-bit data size.
#    define D8BIT 3 ///< 8-bit data size.

/**
 * @brief Handshake options (RTS/CTS).
 */
#    define HAND_NO 0  ///< No handshake.
#    define HAND_YES 1 ///< Handshake enabled (RTS/CTS).

/**
 * @brief UART pin configuration.
 */
#    define UART_TX_PIN 12  ///< UART transmit pin.
#    define UART_RX_PIN 13  ///< UART receive pin.
#    define UART_CTS_PIN 14 ///< UART CTS pin.
#    define UART_RTS_PIN 15 ///< UART RTS pin.

    void enable_uart(uint8_t rts_cts);
    void disable_uart(uint8_t mode);
    void set_default_serial(void);
    void set_uart_protocol(uint8_t cfg_uart, char* resultstr);
    uint8_t get_uart_protocol(char* resultstr);

#    ifdef DEBUG_CODE
    void test_serial_command(void);
#    endif

#    ifdef __cplusplus
}
#    endif

#endif // _SERIAL_H_
