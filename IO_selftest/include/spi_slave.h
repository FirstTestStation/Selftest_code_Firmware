/**
 * @file    spi_slave.h
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Header function to SPI communication
 *
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * @note This file requires <stdint.h> for fixed-width integer types.

 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */
#include <stdint.h>

#ifndef _SPI_SLAVE_H_
#define _SPI_SLAVE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Default SPI configuration.
 */
#define SPI_PORT spi0            ///< SPI instance to be used.
#define PICO_SLAVE_SPI_SCK_PIN 2 ///< SPI clock pin.
#define PICO_SLAVE_SPI_TX_PIN 4  ///< SPI transmit pin (MOSI).
#define PICO_SLAVE_SPI_RX_PIN 3  ///< SPI receive pin (MISO).
#define PICO_SLAVE_SPI_CSN_PIN 5 ///< SPI chip select pin.

/**
 * @brief Default SPI settings.
 */
#define DEF_SPI_STATUS 0  ///< Default SPI status (disabled).
#define DEF_SPI_DATABIT 0 ///< Default data bits (8-bit transfer).
#define DEF_SPI_MODE 0    ///< Default SPI mode.
#define DEF_BAUDRATE 10   ///< Default baud rate (1 MHz).

/**
 * @brief SPI read/write length.
 */
#define SPI_RW_LEN 16 ///< Length for SPI read/write operations.

    void set_default_spi(void);
    void set_spi_com_format(void);
    void enable_spi(void);
    void disable_spi(uint8_t mode);
    void set_spi_protocol(uint8_t cfg_spi, char* resultstr);
    uint8_t get_spi_protocol(char* resultstr);
    void spi_string_protocol(char* protocol_string);

#ifdef DEBUG_CODE
    void test_spi_command(void);
#endif

#ifdef __cplusplus
}
#endif

#endif //
