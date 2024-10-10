/**
 * @file    serial.c
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   function who control the UART serial communication port
 *
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "include/serial.h"
#include "include/selftest.h"


/**
 * @brief Structure containing the serial configuration byte.
 *
 * This structure holds the configuration settings for handshake, stop bits, 
 * data bits, parity, and baud rate in a bitfield format.
 */
struct cfgu
{
    unsigned handshake : 1;  ///< Bit 0: Handshake (0 = no, 1 = yes).
    unsigned stop : 1;       ///< Bit 1: Stop bits (0 = 1 stop bit, 1 = 2 stop bits).
    unsigned databit : 2;    ///< Bits 3-2: Data bits (00 = 5 bits, 01 = 6 bits, 10 = 7 bits, 11 = 8 bits).
    unsigned parity : 2;     ///< Bits 5-4: Parity (00 = none, 01 = even, 10 = odd).
    unsigned baudrate : 2;   ///< Bits 7-6: Baud rate (00 = 9.6k, 01 = 38.4k, 10 = 57.6k, 11 = 115.2k).
};


/**
 * @brief Union to manipulate the configuration byte as bits or bytes.
 *
 * This union allows access to the UART protocol configuration as either
 * a bit field or a byte, enabling flexible manipulation of individual settings.
 */
union uartc
{
    struct cfgu utc;    ///< Bit field of UART protocol configuration.
    uint8_t config;     ///< Byte representation of UART protocol configuration.
};


union uartc serial;  ///< global variable who contains configuration

/**
 * @brief RX interrupt handler work in loopback mode, the character received is transmit back
 *        to the caller
 *
 */
void on_uart_rx()
{
  MESSAGE rec;

  sprintf(&rec.data[0], "\nSerial Interrupt Received: ");
  enque(&rec);

  while (uart_is_readable(UART_ID))
  {
    uint8_t ch = uart_getc(UART_ID);
    // send back the data
    sprintf(&rec.data[0], "receive: %c \n",ch);
    enque(&rec);
    for (size_t i = 0; i < 1000; i++)
    {
      ch++;
      ch--;
    }  // delay to simulate slow terminal
    if (uart_is_writable(UART_ID))
    {
      uart_putc(UART_ID, ch);
    }
  }
}

/**
 * @brief  Fill structure with the default uart configuration
 *
 */
void set_default_serial()
{
  serial.utc.baudrate = SP115_2K;   // speed set to 115200
  serial.utc.parity = UPAR_NONE;    // Parity is set to none
  serial.utc.databit = D8BIT;       // 8 data bits
  serial.utc.stop = STOP1;          // 1 stop bits
  serial.utc.handshake = HAND_YES;  // RTS/CTS enabled
}

/**
 * @brief function who enable the uart and setup RX interrupt
 *
 * @param rts_cts  Enable RTS-CTS handshake if rts_cts = 1
 */
void enable_uart(uint8_t rts_cts)
{
  uint baud_set[4] = {19200, 38400, 57600, 115200};

  int br = baud_set[serial.utc.baudrate];

  // Set up our UART with a basic baud rate.
  uart_init(UART_ID, br);

  // Set the TX and RX pins by using the function select on the GPIO
  // Set datasheet for more information on function select
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  if (rts_cts)
  {  // if handshake required
    gpio_set_function(UART_CTS_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RTS_PIN, GPIO_FUNC_UART);
  }

  // Actually, we want a different speed
  // The call will return the actual baud rate selected, which will be as close as
  // possible to that requested
  int __unused actual = uart_set_baudrate(UART_ID, br);

  // Set UART flow control CTS/RTS,
  bool hand_sk = rts_cts;
  serial.utc.handshake = rts_cts;  // save status in bit field
  uart_set_hw_flow(UART_ID, hand_sk, hand_sk);

  // Set our data format
  uint8_t db = serial.utc.databit + 5;
  uint8_t sb = serial.utc.stop + 1;
  uint8_t pb = serial.utc.parity;

  uart_set_format(UART_ID, db, sb, pb);

  // uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

  // Turn off FIFO's - we want to do this character by character
  uart_set_fifo_enabled(UART_ID, true);

  // Set up a RX interrupt
  // We need to set up the handler first
  // Select correct interrupt for the UART we are using
  int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

  // And set up and enable the interrupt handlers
  irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
  irq_set_enabled(UART_IRQ, true);

  // Now enable the UART to send interrupts - RX only
  uart_set_irq_enables(UART_ID, true, false);
}

/**
 * @brief function who disable the uart and setup uart pin to GPIO
 *
 * @param mode  Set Gpio pin as input(0) or output(1)
 */
void disable_uart(uint8_t mode)
{
  // set pins used for uart to GPIO mode
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_SIO);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_SIO);
  gpio_set_function(UART_CTS_PIN, GPIO_FUNC_SIO);
  gpio_set_function(UART_RTS_PIN, GPIO_FUNC_SIO);

  if (mode == 1)
  {  // if gpio defined as output, set output to 0
    gpio_put(UART_TX_PIN, 0);
    gpio_put(UART_RX_PIN, 0);
    gpio_put(UART_CTS_PIN, 0);
    gpio_put(UART_RTS_PIN, 0);
  }

  gpio_set_dir(UART_TX_PIN, mode);   // set pins as input if mode = 0
  gpio_set_dir(UART_RX_PIN, mode);   // set pins as output if mode = 1
  gpio_set_dir(UART_CTS_PIN, mode);  // set pins as input
  gpio_set_dir(UART_RTS_PIN, mode);  // set pins as input

  // Set UART flow control CTS/RTS, we don't want these, so turn them off
  uart_set_hw_flow(UART_ID, false, false);

  // Turn off FIFO's - we want to do this character by character
  uart_set_fifo_enabled(UART_ID, false);

  // Set up a RX interrupt
  // We need to set up the handler first
  // Select correct interrupt for the UART we are using
  int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
  // disable interrupts
  irq_set_enabled(UART_IRQ, false);

  // Now enable the UART to send interrupts - RX only
  uart_set_irq_enables(UART_ID, false, false);
}

/**
 * @brief From uart protocol byte, build a debug string
 *
 * @param protocol_string string to return to the caller
 */
void uart_string_protocol(char* protocol_string, bool set)
{
  char par;
  const char* ans;

  // extract info to print on debug port

  uint8_t br = serial.utc.baudrate;
  uint8_t pb = serial.utc.parity;
  uint8_t db = serial.utc.databit + 5;
  uint8_t sb = serial.utc.stop + 1;
  uint8_t hk = serial.utc.handshake;

  uint baud_set[4] = {19200, 38400, 57600, 115200};

  if (pb == 0){ par = 'N'; }
  if (pb == 1){ par = 'E'; }
  if (pb == 2){ par = 'O'; }

  ans = (hk ? "YES" : "NO");

  sprintf(protocol_string, "Config uart is [speed:parity:databit:stop:handshake] = [%d,%c,%d,%d,%s]", baud_set[br], par, db, sb, ans);
  if (set)
  {  // if set, program the new format
    uart_set_format(UART_ID, db, sb, pb);
  }
}

/**
 * @brief Set the uart protocol structure with value received externally
 *
 * @param cfg_uart  One byte who define thr uart prtocol to use
 * @param resultstr  return a string with the protocol used on uart
 */
void set_uart_protocol(uint8_t cfg_uart, char* resultstr)
{
  serial.config = cfg_uart;               // save config in structure
  uart_string_protocol(resultstr, true);  // get string of uart protocol
}

/**
 * @brief Get the uart protocol structure with value received externally
 *
 * @param resultstr  return a string with the protocol used on uart
 */
uint8_t get_uart_protocol(char* resultstr)
{
  uart_string_protocol(resultstr, false);  // get string of uart protocol
  return serial.config;
}

/**
 * @brief test command to validate the command function
 *        used only in development of firmware
 *
 */

#ifdef DEBUG_CODE
void test_serial_command()
{
  send_master(105, 0);           // test command get uart protocol
  send_master(102, 1);           // test command Disable uart
  send_master(75, 12);           // test command
  send_master(75, 13);           // test command
  send_master(75, 14);           // test command
  send_master(75, 15);           // test command
  send_master(103, 0b01000110);  // test command uart protocol
  send_master(105, 0);           // test command get uart protocol
  send_master(101, 0);           // test command    enable uart without handshake
  send_master(75, 12);           // test command
  send_master(75, 13);           // test command
  send_master(75, 14);           // test command
  send_master(75, 15);           // test command
  send_master(101, 1);           // test command    Enable uart with handshake
  send_master(75, 12);           // test command
  send_master(75, 13);           // test command
  send_master(75, 14);           // test command
  send_master(75, 15);           // test command
}
#endif