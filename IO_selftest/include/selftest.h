/*
 * 
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _SELFTEST_H_
#define _SELFTEST_H_



#ifdef __cplusplus
extern "C" {
#endif


#define DEBUG  // DEbug mode
#define DEBUG_UART  // DEbug mode for test uart


// For help debug code on selftest board,  a loopback has been made using gpio reserved 
// for jtag testing. If debug is defined, the slave i2C pin are modified to be connected
// on the loopback


#ifdef DEBUG_UART
    #define  I2C_SLAVE_SDA_PIN   18 
    #define  I2C_SLAVE_SCL_PIN   19 
#endif

#ifndef DEBUG_UART
    #define  I2C_SLAVE_SDA_PIN   6 
    #define  I2C_SLAVE_SCL_PIN   7 
#endif

#define  I2C_MASTER_SDA_PIN 16 
#define  I2C_MASTER_SCL_PIN 17 


#define MESSAGE_SIZE 80
#define QUEUE_SIZE 64 // set high for development


typedef struct {
char data[MESSAGE_SIZE];
} MESSAGE;

struct {   // global structure
    MESSAGE messages[QUEUE_SIZE];
    int begin;
    int end;
    int current_load;
} queue;


bool enque(MESSAGE *message);
void send_master(uint8_t cmd, uint16_t wdata);

#ifdef __cplusplus
}
#endif

#endif //

