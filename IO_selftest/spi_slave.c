/**
 * @file    spi_slave.c
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   function who control SPI communication port in slave mode
 *
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "hardware/irq.h"
#include "include/spi_slave.h"
#include "hardware/spi.h"
#include "include/selftest.h"

uint8_t rbytes[255];   ///< SPI register for byte data (up to 255 bytes).
uint16_t rwords[255];  ///< SPI register for word data (up to 255 words).

/**
 * @brief Structure containing the SPI configuration byte.
 *
 * This structure holds the configuration settings for the SPI protocol,
 * including status, mode, data bits, and baud rate in a bitfield format.
 */
struct cfgspi
{
  unsigned status : 1;    ///< Bit 0: Status (0 = disabled, 1 = enabled).
  unsigned mode : 2;      ///< Bits 2-1: SPI mode (00 = mode 0, 01 = mode 1, 10 = mode 2, 11 = mode 3).
  unsigned databit : 1;   ///< Bit 3: Data bit size (0 = 8 bits, 1 = 16 bits).
  unsigned baudrate : 4;  ///< Bits 7-4: Baud rate (0-15 representing different speeds).
};

/**
 * @brief Union to manipulate the SPI configuration byte as bits or bytes.
 *
 * This union allows access to the SPI protocol configuration as either
 * a bit field or a byte, enabling flexible manipulation of individual settings.
 */
union spi_stc
{
  struct cfgspi stc;  ///< Bit field of SPI protocol configuration.
  uint8_t config;     ///< Byte representation of SPI protocol configuration.
};

union spi_stc spi;  ///< Global variable containing the SPI configuration.

/**
 * @brief  Fill structure with the default spi configuration
 *
 */
void set_default_spi()
{
  spi.stc.baudrate = DEF_BAUDRATE;    ///  Set default Baudrate
  spi.stc.mode = DEF_SPI_MODE;        /// Set default mode.
  spi.stc.databit = DEF_SPI_DATABIT;  /// Set default data bit size.
  spi.stc.status = DEF_SPI_STATUS;    /// Disable by default
}

// byte memory for SPI read-write
static uint8_t in_b_buf[SPI_RW_LEN], out_b_buf[SPI_RW_LEN];
// Word memory for SPI read-write
static uint16_t in_w_buf[SPI_RW_LEN], out_w_buf[SPI_RW_LEN];

/**
 * @brief spi slave receiver interrupt
 *
 *  Two interrupts are received by command write_read
 *  The first interrupt read the the data from source and the second interrupts
 *  write reverse data to the source
 *
 */
void spi_slave_rx_interrupt_handler()
{
  MESSAGE rec;
  int x = 0;  // number of character received in read portion

  sprintf(&rec.data[0], "\nSPI Slave Interrupt Received");
  enque(&rec);

  // loop to read each character

  if (spi.stc.databit == 0)
  {  // if 8 bits read and write
    // receive byte from master and echo back the reverse on next interrupt
    while (spi_is_readable(SPI_PORT))
    {
      spi_write_read_blocking(SPI_PORT, &out_b_buf[x], &in_b_buf[x], 1);
      x++;
    }

    // After data received, prepare data to transmit on next interrupt
    for (int k = 0; k < x; k++)
    {
      sprintf(&rec.data[0], "SPI byte Data # %d, read: 0x%x, write: 0x%x", k, in_b_buf[k], out_b_buf[k]);
      enque(&rec);

      if (in_b_buf[k] != 0)
      {                               // if data read is valid
        out_b_buf[k] = ~in_b_buf[k];  // set value for read data portion
      }
    }
  }
  else
  {  // if 16 bits
     // receive byte from master and echo back the reverse on next interrupt
    while (spi_is_readable(SPI_PORT))
    {
      spi_write16_read16_blocking(SPI_PORT, &out_w_buf[x], &in_w_buf[x], 1);
      x++;
    }

    for (int k = 0; k < x; k++)
    {
      sprintf(&rec.data[0], "SPI word Data # %d, read: 0x%x, write: 0x%x", k, in_w_buf[k], out_w_buf[k]);
      enque(&rec);

      if (in_w_buf[k] != 0)
      {                               // if data read is valid
        out_w_buf[k] = ~in_w_buf[k];  // set reverse value to write portion
      }
    }
  }
}

/**
 * @brief Set the up spi slave object
 *        Use interrupt to send test data to the master
 *
 */
void enable_spi(void)
{
  MESSAGE rec;

  // Enable SPI 0
  spi_init(SPI_PORT, spi.stc.baudrate * 100E3);  // not required for slave SPI
  gpio_set_function(PICO_SLAVE_SPI_RX_PIN, GPIO_FUNC_SPI);
  gpio_set_function(PICO_SLAVE_SPI_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(PICO_SLAVE_SPI_TX_PIN, GPIO_FUNC_SPI);
  gpio_set_function(PICO_SLAVE_SPI_CSN_PIN, GPIO_FUNC_SPI);
  gpio_set_dir(PICO_SLAVE_SPI_CSN_PIN, 0);  // input
  spi_set_slave(SPI_PORT, true);
  set_spi_com_format();

  // Attach the interrupt handler
  irq_set_exclusive_handler(SPI0_IRQ, spi_slave_rx_interrupt_handler);

  // Enable the RX FIFO interrupt   (RXIM)
  spi0_hw->imsc = SPI_SSPIMSC_RTIM_BITS | SPI_SSPIMSC_RORIM_BITS | SPI_SSPIMSC_RXIM_BITS;

  // Enable the SPI interrupt
  irq_set_enabled(SPI0_IRQ, 1);

  spi.stc.status = 1;  // Set flag to indicate of spi is enabled

  for (int i = 0; i < SPI_RW_LEN; i++)
  {  // initialize write buffer with value
    out_b_buf[i] = i | (i << 4);
    out_w_buf[i] = i | (i << 4) | (i << 8) | (i << 12);
  }

  sprintf(&rec.data[0], "Selftest SPI is Enabled\r\n");
  enque(&rec);
}

/**
 * @brief function who disable the uart and setup uart pin to GPIO
 *
 * @param mode  Set Gpio pin as input(0) or output(1)
 */
void disable_spi(uint8_t mode)
{
  // Disable.
  spi_deinit(SPI_PORT);

  gpio_set_function(PICO_SLAVE_SPI_RX_PIN, GPIO_FUNC_SIO);
  gpio_set_function(PICO_SLAVE_SPI_SCK_PIN, GPIO_FUNC_SIO);
  gpio_set_function(PICO_SLAVE_SPI_TX_PIN, GPIO_FUNC_SIO);
  gpio_set_function(PICO_SLAVE_SPI_CSN_PIN, GPIO_FUNC_SIO);

  gpio_set_dir(PICO_SLAVE_SPI_RX_PIN, mode);   // set pins as input if mode = 0
  gpio_set_dir(PICO_SLAVE_SPI_SCK_PIN, mode);  // set pins as output if mode = 1
  gpio_set_dir(PICO_SLAVE_SPI_TX_PIN, mode);   // set pins as input
  gpio_set_dir(PICO_SLAVE_SPI_CSN_PIN, mode);  // set pins as input

  // Disable the SPI interrupt
  irq_set_enabled(SPI0_IRQ, 0);

  // Detach the interrupt handler
  irq_remove_handler(SPI0_IRQ, spi_slave_rx_interrupt_handler);

  spi.stc.status = 0;  // Reset flag to indicate of serial port is disabled
  fprintf(stdout, "Selftest SPI is disabled\r\n");
}

/**
 * @brief function to set the mode to use for spi communication
 *
 */
void set_spi_com_format()
{
  bool cpol, cpha, msb;

  uint8_t databits = (spi.stc.databit == 0 ? 8 : 16);
  msb = SPI_MSB_FIRST;  // LSB First is not supported

  switch (spi.stc.mode)
  {
    case 0:
    {
      cpol = SPI_CPOL_0;
      cpha = SPI_CPHA_0;
      break;
    }
    case 1:
    {
      cpol = SPI_CPOL_0;
      cpha = SPI_CPHA_1;
      break;
    }
    case 2:
    {
      cpol = SPI_CPOL_1;
      cpha = SPI_CPHA_0;
      break;
    }
    case 3:
    {
      cpol = SPI_CPOL_1;
      cpha = SPI_CPHA_1;
      break;
    }
  }

  // set SDPI format
  spi_set_format(SPI_PORT, databits, cpol, cpha, msb);
  fprintf(stdout, "SPI Format,  Databit = %d, Mode = %d, define: Cpol = %d, Cpha = %d, Msb = %d\r\n", databits, spi.stc.mode, cpol, cpha, msb);
}

/**
 * @brief Set the SPI protocol structure with value received externally
 *
 * @param cfg_spi   One byte who define the spi protocol to use
 * @param resultstr  return a string with the protocol used on uart
 */
void set_spi_protocol(uint8_t cfg_spi, char* resultstr)
{
  uint8_t status = spi.stc.status;  // save actual status
  spi.config = cfg_spi;             // save config in structure, status is in read only
  spi.stc.status = status;          // replace value read before actualization

  spi_string_protocol(resultstr);  // get string of spi protocol
}

/**
 * @brief Get the spi protocol structure with value received externally
 *
 * @param resultstr  return a string with the protocol used on uart
 */
uint8_t get_spi_protocol(char* resultstr)
{
  spi_string_protocol(resultstr);  // get string of uart protocol
  return spi.config;
}

/**
 * @brief From spi protocol byte, build a debug string
 *
 * @param protocol_string string to return to the caller
 */
void spi_string_protocol(char* protocol_string)
{
  const char* ans;
  uint8_t db;

  // extract info to print on debug port

  uint8_t br = spi.stc.baudrate;
  uint8_t md = spi.stc.mode;

  db = (spi.stc.databit == 0 ? 8 : 16);
  ans = (spi.stc.status == 0 ? "DIS" : "ENA");

  sprintf(protocol_string, "Config SPI is [speed(x 100KHz):mode:databit:status:] = [%d,%d,%d,%s]", br, md, db, ans);
}

/**
 * @brief test command to validate the command function
 *        used only in development of firmware
 *
 */
#ifdef DEBUG_CODE
void test_spi_command()
{
  fprintf(stdout, "SPI Test Command\r\n");

  send_master(115, 0);           // test command get spi protocol
  send_master(112, 1);           // test command Disable spi
  send_master(75, 2);            // test command mode (SOI or SPI?)
  send_master(75, 3);            // test command
  send_master(75, 4);            // test command
  send_master(75, 5);            // test command
  send_master(113, 0b10100001);  // test command spi protocol
  send_master(115, 0);           // test command get spi protocol
  send_master(111, 0);           // test command    enable uart without handshake
  send_master(75, 2);            // test command
  send_master(75, 3);            // test command
  send_master(75, 4);            // test command
  send_master(75, 5);            // test command
  send_master(113, 0b00100110);  // test command spi protocol
  send_master(115, 0);           // test command get spi protocol
}
#endif
