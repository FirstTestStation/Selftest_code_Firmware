/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "include/serial.h"
#include "include/selftest.h"


struct cfgu {
    unsigned handshake: 1;      // Bit 0
    unsigned stop : 1 ;         // Bit 1
    unsigned databit : 2 ;      // Bit 3-2
    unsigned parity : 2 ;       // Bit 5-4
    unsigned baudrate : 2 ;     // Bit 7-6
};

union uartc {
    struct cfgu utc;        // bit field of uart protocol
    uint8_t config;         // byte field of uart protcvol
};



union uartc serial; // global variable who contains configuration


/**
 * @brief RX interrupt handler work in loopback mode, the character received is transmit back
 *        to the caller
 * 
 */
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        volatile uint8_t ch = uart_getc(UART_ID);
        //send back the data
        for (size_t i = 0; i <2000; i++) { ch++;ch--;} //delay to simulate slow terminal
        if (uart_is_writable(UART_ID)) {
            uart_putc(UART_ID, ch);
            
        }
    }
}

/**
 * @brief  Fill structure with the default uart configuration
 * 
 */
void set_default_serial(){

    serial.utc.baudrate= SP115_2K;   // speed set to 115200
    serial.utc.parity= UPAR_NONE;    // Parity is set to none
    serial.utc.databit= D8BIT;          // 8 data bits
    serial.utc.stop= STOP1;          // 1 stop bits
    serial.utc.handshake = HAND_YES;  // RTS/CTS enabled

}

/**
 * @brief function who enable the uart and setup RX interrupt
 *
 * @param rts_cts  Enable RTS-CTS handshake if rts_cts = 1 
 */
void enable_uart(uint8_t rts_cts) {

    uint baud_set[4] = {19200,38400,57600,115200};

    int br = baud_set[serial.utc.baudrate];

    // Set up our UART with a basic baud rate.
    uart_init(UART_ID, br);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    if (rts_cts) { // if handshake required
        gpio_set_function(UART_CTS_PIN, GPIO_FUNC_UART);
        gpio_set_function(UART_RTS_PIN, GPIO_FUNC_UART);
    }


    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set UART flow control CTS/RTS,
    bool handsk = rts_cts;
    serial.utc.handshake= rts_cts;  // save status in bit field 
    uart_set_hw_flow(UART_ID, handsk, handsk);

    // Set our data format
    uint8_t db = serial.utc.databit + 5;
    uint8_t sb = serial.utc.stop;
    uint8_t pb = serial.utc.parity;

    uart_set_format(UART_ID, db,sb,pb);;

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
void disable_uart(uint8_t mode) {

    // set pins used for uart to GPIO mode
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_SIO);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_SIO);
    gpio_set_function(UART_CTS_PIN, GPIO_FUNC_SIO);
    gpio_set_function(UART_RTS_PIN, GPIO_FUNC_SIO);

    if (mode ==1) {  // if gpio defined as output, set output to 0 
        gpio_put(UART_TX_PIN,0);
        gpio_put(UART_RX_PIN,0);
        gpio_put(UART_CTS_PIN,0);
        gpio_put(UART_RTS_PIN,0);
    }

    
    gpio_set_dir (UART_TX_PIN,mode); // set pins as input if mode = 0
    gpio_set_dir (UART_RX_PIN,mode); // set pins as output if mode = 1
    gpio_set_dir (UART_CTS_PIN,mode); // set pins as input
    gpio_set_dir (UART_RTS_PIN,mode); // set pins as input

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
void uart_string_protocol(char *protocol_string) {
    char par;
    const char *ans;


 // extract info to print on debug port

    volatile uint8_t br = serial.utc.baudrate;
    volatile uint8_t pb = serial.utc.parity;
    volatile uint8_t db = serial.utc.databit + 5;
    volatile uint8_t sb = serial.utc.stop;
    volatile uint8_t hk = serial.utc.handshake;

    uint baud_set[4] = {19200,38400,57600,115200};

    if (pb == 0) {par = 'N'; }
    if (pb == 1) {par = 'E'; }
    if (pb == 2) {par = 'O'; }

    ans = (hk? "YES":"NO");

    sprintf(protocol_string,"Config uart is [speed:parity:databit:stop:handshake] = [%d,%c,%d,%d,%s]",\
    baud_set[br], par,db,sb,ans);

}

/**
 * @brief Set the uart protocol structure with value received externally
 * 
 * @param cfg_uart  One byte who define thr uart prtocol to use
 * @param resulstr  return a string with the protocol used on uart
 */
void set_uart_protocol(uint8_t cfg_uart,char *resultstr) {

    serial.config = cfg_uart;  // save config in structure
    uart_string_protocol(resultstr); // get string of uart protocol  
}

/**
 * @brief Get the uart protocol structure with value received externally
 * 
 * @param resulstr  return a string with the protocol used on uart
 */
uint8_t get_uart_protocol(char *resultstr) {

    uart_string_protocol(resultstr); // get string of uart protocol
    return serial.config;  

}

/**
 * @brief test command to validate the command function
 *        used only in development of firmware
 * 
 */

#ifdef DEBUG_UART 
void test_serial_command(){

        send_master (105,0); // test command get uart protocol
        send_master (102,1); // test command Disable uart
        send_master (75,12); // test command
        send_master (75,13); // test command
        send_master (75,14); // test command 
        send_master (75,15); // test command
        send_master (103,0b01000110); // test command uart protocol
        send_master (105,0); // test command get uart protocol
        send_master (101,0); // test command    enable uart without handshake
        send_master (75,12); // test command
        send_master (75,13); // test command
        send_master (75,14); // test command 
        send_master (75,15); // test command
        send_master (101,1); // test command    Enable uart with handshake
        send_master (75,12); // test command
        send_master (75,13); // test command
        send_master (75,14); // test command 
        send_master (75,15); // test command

    }
  #endif