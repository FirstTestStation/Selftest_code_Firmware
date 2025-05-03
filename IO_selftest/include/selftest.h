/**
 * @file    selftest.h
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Header function to main selftest loop
 *
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#include <stdbool.h>
#include <stdint.h>

#ifndef _SELFTEST_H_
#    define _SELFTEST_H_

#    ifdef __cplusplus
extern "C"
{
#    endif

    //#define DEBUG_CODE  // DEbug mode for test code using i2c loopback

    // For help debug code on selftest board,  a loopback has been made using gpio reserved
    // for JTAG testing. If debug is defined, the slave i2C pin are modified to be connected
    // on the loopback

#    ifdef DEBUG_CODE
/**
 * @brief Pins used for I2C loopback mode during debugging.
 */
#        define I2C_SLAVE_SDA_PIN 18  /**< SDA pin for I2C slave in loopback mode. */
#        define I2C_SLAVE_SCL_PIN 19  /**< SCL pin for I2C slave in loopback mode. */
#        define I2C_MASTER_SDA_PIN 16 /**< SDA pin for I2C master in loopback mode. */
#        define I2C_MASTER_SCL_PIN 17 /**< SCL pin for I2C master in loopback mode. */
#    endif

#    ifndef DEBUG_CODE
/**
 * @brief Pins used for I2C in normal operation.
 */
#        define I2C_SLAVE_SDA_PIN 6 /**< SDA pin for I2C slave in normal operation. */
#        define I2C_SLAVE_SCL_PIN 7 /**< SCL pin for I2C slave in normal operation. */
#    endif

#    define MESSAGE_SIZE 120          ///< Size of each message.
#    define QUEUE_SIZE 255            ///< Queue size, set high for development.
#    define WATCHDOG_TIMEOUT_MS 10000 ///< Watchdog timeout (10 seconds).
#    define GPIOF 10                  ///< GPIO pin used to generate frequency output.

    /**
     * @brief Structure representing a message.
     */
    typedef struct
    {
        char data[MESSAGE_SIZE]; ///< Data contained in the message.
    } MESSAGE;

    bool enque(MESSAGE* message);
    void set_pwm_frequency(bool setpwm, uint8_t sfreq);

#    ifdef __cplusplus
}
#    endif

#endif //
