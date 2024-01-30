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

#define SPI_PORT                spi0
#define PICO_SLAVE_SPI_SCK_PIN   2
#define PICO_SLAVE_SPI_TX_PIN    4
#define PICO_SLAVE_SPI_RX_PIN    3
#define PICO_SLAVE_SPI_CSN_PIN   5
#define SPI_RW_LEN  0x1

void setup_spi_slave(void);

#ifdef __cplusplus
}
#endif

#endif //

