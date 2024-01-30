/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "hardware/irq.h"
#include "include/spi_slave.h"
#include "hardware/spi.h"
#include "include/selftest.h"

/**
 * @brief spi slave receiver interrupt
 * 
 */
void spi_slave_rx_interrupt_handler() {
  MESSAGE rec; 
  uint8_t in_buf[SPI_RW_LEN]; // buffer to receive the character

  int nb = spi_read_blocking (spi_default, 0, in_buf, SPI_RW_LEN);
  spi_write_blocking(spi_default, in_buf, SPI_RW_LEN);
  sprintf(&rec.data[0],"SPI Slave Received nb_char: %d, Data 0x%x",nb,in_buf[0]);
  enque(&rec);
}


void setup_spi_slave(void){

 // Enable SPI 0 at 1 MHz and connect to GPIOs
 spi_init(SPI_PORT , 1000 * 1000);
 spi_set_slave(SPI_PORT, true);
 gpio_set_function(PICO_SLAVE_SPI_RX_PIN, GPIO_FUNC_SPI);
 gpio_set_function(PICO_SLAVE_SPI_SCK_PIN, GPIO_FUNC_SPI);
 gpio_set_function(PICO_SLAVE_SPI_TX_PIN, GPIO_FUNC_SPI);
 gpio_set_function(PICO_SLAVE_SPI_CSN_PIN, GPIO_FUNC_SPI);
 gpio_set_dir(PICO_SLAVE_SPI_CSN_PIN,0);  // input
 

 // Enable the RX FIFO interrupt (RXIM)
 spi0_hw->imsc = 1 << 2;

 // Enable the SPI interrupt
  irq_set_enabled (SPI0_IRQ,1);

  // Attach the interrupt handler
  irq_set_exclusive_handler (SPI0_IRQ, spi_slave_rx_interrupt_handler);
}
