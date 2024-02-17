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


static uint8_t in_buf[SPI_RW_LEN],out_buf[SPI_RW_LEN] ;


/**
 * @brief spi slave receiver interrupt
 * 
 */
void spi_slave_rx_interrupt_handler() {
  MESSAGE rec; 

  sprintf(&rec.data[0],"SPI Slave Interrupt Received");
  enque(&rec);

  // receive character from master and echo back the reverse on next interrupt
  spi_write_read_blocking(SPI_PORT, out_buf, in_buf, SPI_RW_LEN);

  out_buf[0] = ~in_buf[0]; // invert data and save on output buffer
  out_buf[1] = ~in_buf[1];
  out_buf[2] = ~in_buf[2];
  out_buf[3] = ~in_buf[3];
  out_buf[4] = ~in_buf[4];
  out_buf[5] = ~in_buf[5];
  out_buf[6] = ~in_buf[6];
  out_buf[7] = ~in_buf[7];

  // save on message queue
  sprintf(&rec.data[0],"SPI Data Received: (first five data only):0x%x,0x%x,0x%x,0x%x,0x%x",in_buf[0],in_buf[1],in_buf[2],in_buf[3],in_buf[4]);
  enque(&rec);

}



/**
 * @brief Set the up spi slave object
 *        Use interrupt to send test data to the master
 * 
 */
void setup_spi_slave(void){

 // Enable SPI 0 at 10Khz 1 MHz and connect to GPIOs
 spi_init(SPI_PORT ,1000 * 1000);
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
