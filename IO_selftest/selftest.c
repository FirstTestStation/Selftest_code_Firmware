/*
 * Copyright (c) 2022 Daniel Lockhead
 *
 * SPDX-License-Identifier: MIT
 */

#include <i2c_fifo.h>
#include <i2c_slave.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "hardware/watchdog.h"
#include "hardware/gpio.h"
#include "userconfig.h"
#include "include/serial.h"
#include "hardware/spi.h"
#include "include/selftest.h"
#include "include/spi_slave.h"



static const uint I2C_OFFSET_ADDRESS = 0x20;  // ofsset to add to the physical address read
//static const uint PICO_PORT_ADDRESS = 0x23;  // Pico address where port is used
static const uint REG_STATUS = 100;  // Register used to report Status


static const uint I2C_BAUDRATE = 100000; // 100 kHz
static const uint I2C_SLAVE_ADDRESS_IO0 = 26;     // Bit 0 of I2C Address
static const uint I2C_SLAVE_ADDRESS_IO1 = 27;     // Bit 1 of I2C Address


// For this example, we run both the master and slave from the same board.
// You'll need to wire pin GP4 to GP6 (SDA), and pin GP5 to GP7 (SCL).
//static const uint I2C_SLAVE_SDA_PIN = 6; //PICO_DEFAULT_I2C_SDA_PIN; // 4
//static const uint I2C_SLAVE_SCL_PIN = 7; //PICO_DEFAULT_I2C_SCL_PIN; // 5
//static const uint I2C_MASTER_SDA_PIN = 20; // used in design
//static const uint I2C_MASTER_SCL_PIN = 21; // used in design

// Define lines as GPIO at boot
static const uint32_t GPIO_BOOT_MASK = 0b00011100010011111111111111111111;
// Define direction of GPIO lines
static const uint32_t GPIO_SET_DIR_MASK = 0b0010000010000000000000000000;  // GPIO MASK
static const uint32_t GPIO_SELF_OUT_MASK = 0x00ul;  // All output to 0
static const uint32_t GPIO_SELF_DIR_MASK = 0b00010000000000000000000000000000; 

//static const uint32_t PORT0_MASK = 0xfful;
//static const uint32_t PORT1_MASK = 0b111111110000000000;
//static const uint     PORT1_OFFSET = 10;  // shift to do for match mask


//static const uint32_t GPIO_BANK0_MASK = 0xfful;  // Bank 0 
//static const uint32_t GPIO_BANK1_MASK = 0b00000000000000111111110000000000;;  // Bank 1 


          

//#define PICO_DEFAULT_UART_TX_PIN 8  // if Serial enabled
//#define PICO_DEFAULT_UART_RX_PIN 9

/*
Portable array-based cyclic FIFO queue.
Copy from Internet
*/

bool enque(MESSAGE *message) {
    if (queue.current_load < QUEUE_SIZE) {
        if (queue.end == QUEUE_SIZE) {
            queue.end = 0;
        }
        queue.messages[queue.end] = *message;
        queue.end++;
        queue.current_load++;
        return true;
    } else {
        return false;
    }
}

void init_queue() {
    queue.begin = 0;
    queue.end = 0;
    queue.current_load = 0;
    memset(&queue.messages[0], 0, QUEUE_SIZE * sizeof(MESSAGE_SIZE));
}


bool deque(MESSAGE *message) {
    if (queue.current_load > 0) {
        *message = queue.messages[queue.begin];
        memset(&queue.messages[queue.begin], 0, sizeof(MESSAGE));
        queue.begin = (queue.begin + 1) % QUEUE_SIZE;
        queue.current_load--;
        return true;
    } else {
        return false;
    }
}

// Error flags collected during execution
static union {
  uint8_t  all_flags;
  struct
  {
  uint8_t cfg:1;
  uint8_t cmd:1;
  uint8_t error:1;
  uint8_t watch:1;
  uint8_t sparesA:1;
  uint8_t sparesB:1;
  uint8_t sparesC:1;
  uint8_t sparesD:1;
  };
 } status;     


// The slave implements a 256 byte memory. 
// The memory address use the command byte value as memory pointer
// The 8 bit data is written starting at command value


static struct
{
    uint8_t reg[128];           // contains data following command byte
    uint8_t reg_address;        // contains command number
    uint8_t reg_status;         // contains status of command 
    bool reg_address_written;   // Flag for command byte received
    uint8_t i2c_add;
} context;

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    MESSAGE rec; 
    uint8_t cmd; // = 127;  // keep command value
    bool tvalue;
    uint8_t svalue;
    uint32_t maskvalue;
    char str_answer[80];

    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
        if (!context.reg_address_written) {
            // writes always start with the memory address
            context.reg_address = i2c_read_byte(i2c);  // Command byte
            context.reg_address_written = true;
            // sprintf(&rec.data[0],"On i2c  cmd");
            //enque(&rec);
        } else {   // WRITE COMMAND
            context.reg[context.reg_address] = i2c_read_byte(i2c); // read Byte
           // sprintf(&rec.data[0],"On i2c  data");
           // enque(&rec);

            cmd = context.reg_address;

            switch(cmd){  // Command byte

                case 10: // Clear Gpio
                  gpio_put(context.reg[context.reg_address] , 0);   
                  sprintf(&rec.data[0],"Cmd %02d, Clear Gpio: %02d ",cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;

                case 11: // Set Gpio
                  gpio_put(context.reg[context.reg_address] , 1);   
                  sprintf(&rec.data[0],"Cmd %02d, Set Gpio: %02d ", cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;

                case 20: // Set Gpio Direction to Output
                  gpio_set_dir(context.reg[context.reg_address] , 1);   // turn OFF Led
                  sprintf(&rec.data[0],"Cmd %02d, Set Dir Out Gpio: %02d ", cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;

                case 21: // Set Gpio Direction to Input
                  gpio_set_dir(context.reg[context.reg_address] , 0);   // turn OFF Led
                  sprintf(&rec.data[0],"Cmd %02d, Set dir In Gpio: %02d ", cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;                  

                case 30: // Set GPIO strenght = 2mA  
                  gpio_set_drive_strength(context.reg[context.reg_address], GPIO_DRIVE_STRENGTH_2MA);   // set Value
                  sprintf(&rec.data[0],"Cmd %02d, 2mA Gpio: %02d ", cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;

                case 31: // Set GPIO strenght = 4mA  
                  gpio_set_drive_strength(context.reg[context.reg_address], GPIO_DRIVE_STRENGTH_4MA);   // set Value
                  sprintf(&rec.data[0],"Cmd %02d, 4mA Gpio: %02d ", cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;
 
                case 32: // Set GPIO strenght = 8mA  
                  gpio_set_drive_strength(context.reg[context.reg_address], GPIO_DRIVE_STRENGTH_8MA);   // set Value
                  sprintf(&rec.data[0],"Cmd %02d, 8mA Gpio: %02d ", cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;
                                   
                case 33: // Set GPIO strenght = 12mA  
                  gpio_set_drive_strength(context.reg[context.reg_address], GPIO_DRIVE_STRENGTH_12MA);   // set Value
                  sprintf(&rec.data[0],"Cmd %02d, 12mA Gpio: %02d ", cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;

                case 41: // Set pull-up
                  gpio_pull_up(context.reg[context.reg_address]);   // turn ON pull-up
                  sprintf(&rec.data[0],"Cmd %02d, Pull-up Gpio: %02d,  ", cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;


                case 50: // Clear pull-up and pull-down
                  gpio_disable_pulls(context.reg[context.reg_address]);   // turn ON pull-up
                  sprintf(&rec.data[0],"Cmd %02d, Clear pull-up, pull-down Gpio: %02d,  ", cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;

                case 51: // Set pull-down
                  gpio_pull_down(context.reg[context.reg_address]);   // turn ON pull-down
                  sprintf(&rec.data[0],"Cmd %02d, Pull-down Gpio: %02d,  ", cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;

                case 60: // Set PAD state, Nothing to do other than save on register
                  sprintf(&rec.data[0],"Cmd %02d, Pad State: %01d ",cmd,  context.reg[context.reg_address]);
                  enque(&rec);
                  break;

                case 61: // Set GPx to PAD state
                  maskvalue = 0xfful;
                  hw_write_masked(&padsbank0_hw ->io[context.reg[context.reg_address]],context.reg[cmd-1],maskvalue); // Set Pad state
                  sprintf(&rec.data[0],"Cmd %02d, Set Pad State to Gpio: %02d ,State: 0x%01x ",cmd,  context.reg[context.reg_address],context.reg[cmd-1]);
                  enque(&rec);
                  break;

                case 101: // Enable Uart TX/RX w/wo RTS/CTS
                  enable_uart(context.reg[context.reg_address]);   // Enable uart
                  sprintf(&rec.data[0],"Cmd %d, Enable UART, handshake RTS/CTS(1): %d ", cmd,context.reg[context.reg_address] );
                  enque(&rec);
                  break;

                case 102: // Disable Uart and set as SIO
                  disable_uart(context.reg[context.reg_address]);   // Disable uart
                  sprintf(&rec.data[0],"Cmd %d, Disable UART, Set GPIO Input(0) Output(1): %d ", cmd,context.reg[context.reg_address]);
                  enque(&rec);
                  break;

                case 103: // Set uart protocol
                  set_uart_protocol(context.reg[context.reg_address],str_answer);   // Set uart protocol
                  sprintf(&rec.data[0],"%s", str_answer);
                  enque(&rec);
                  break;


 
            }              
        }
        break;

    case I2C_SLAVE_REQUEST: // master is requesting data
        // load from register
            cmd = context.reg_address;

            // For command requesting a Get Value, The register is updated before return the content
            // For readback of Set value, we just return the contents of register

            switch(cmd){  // Command byte yo et register

                case 01: // get Major Version
                  context.reg[context.reg_address] = IO_SELFTEST_VERSION_MAJOR;
                  sprintf(&rec.data[0],"Cmd %02d, MAJ Version: %02d ", cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;

                case 02: // get Minor Version
                  context.reg[context.reg_address] = IO_SELFTEST_VERSION_MINOR;
                  sprintf(&rec.data[0],"Cmd %02d, MIN Version: %02d ", cmd, context.reg[context.reg_address]);
                  enque(&rec);
                  break;


                case 15: // read True value of Gpio
                  tvalue = gpio_get(context.reg[context.reg_address]);   // Read true Value
                  sprintf(&rec.data[0],"Cmd %02d, read True Gpio: %02d ,State: %01d ", cmd, context.reg[context.reg_address],tvalue);
                  enque(&rec);
                  context.reg[context.reg_address] = tvalue;
                  break;


                case 25: // get GPIO Direction
                  tvalue = gpio_get_dir(context.reg[context.reg_address]);   // Read Direction Value
                  sprintf(&rec.data[0],"Cmd %02d, Red Dir Gpio: %02d ,State: %01d ", cmd, context.reg[context.reg_address],tvalue);
                  enque(&rec);
                  context.reg[context.reg_address] = tvalue;
                  break;

                case 35: // get GPIO strenght
                  svalue =  gpio_get_drive_strength(context.reg[context.reg_address]);   // Read strenght Value
                  sprintf(&rec.data[0],"Cmd %02d, Read strenght Gpio: %02d ,State: %01d ",cmd,  context.reg[context.reg_address],svalue);
                  enque(&rec);
                  context.reg[context.reg_address] = svalue;
                  break;

                case 45: // get pull-up
                  tvalue = gpio_is_pulled_up(context.reg[context.reg_address]);   // Read true Value
                  sprintf(&rec.data[0],"Cmd %02d, read pull-up Gpio: %02d ,State: %01d ", cmd, context.reg[context.reg_address],tvalue);
                  enque(&rec);
                  context.reg[context.reg_address] = tvalue;
                  break;

                case 55: // get pull-down
                  tvalue = gpio_is_pulled_down(context.reg[context.reg_address]);   // Read true Value
                  sprintf(&rec.data[0],"Cmd %02d, Read pull-down Gpio: %02d ,State: %01d ", cmd, context.reg[context.reg_address],tvalue);
                  enque(&rec);
                  context.reg[context.reg_address] = tvalue;
                  break;

                case 65: // get PAD state
                  svalue =  padsbank0_hw ->io[context.reg[context.reg_address]] &0xff; // Read gpio PAD Value
                  sprintf(&rec.data[0],"Cmd %02d, Gpio: %02d ,Read PAD State: 0x%01x ",cmd,  context.reg[context.reg_address],svalue);
                  enque(&rec);
                  context.reg[context.reg_address] = svalue;
                  break;

                case 75: // get GPIO function
                  svalue =  gpio_get_function(context.reg[context.reg_address]);   // Read function
                  sprintf(&rec.data[0],"Cmd %02d, Read function Gpio: %02d , funct: 0x%02x ",cmd,  context.reg[context.reg_address],svalue);
                  enque(&rec);
                  context.reg[context.reg_address] = svalue;
                  break;


                case 100: // get statsus register, nothing to do
                  context.reg[REG_STATUS] = status.all_flags;
                  sprintf(&rec.data[0],"Cmd %02d,Status register: 0x%01x ",cmd,  context.reg[REG_STATUS]);
                  enque(&rec);
                  break;

                case 105: // get UART protocol
                  svalue = get_uart_protocol(str_answer);   // Get uart protocol
                  sprintf(&rec.data[0],"%s", str_answer);
                  enque(&rec);
                  context.reg[context.reg_address] = svalue;
                  break;

                

            }


        i2c_write_byte(i2c, context.reg[context.reg_address]);
        sprintf(&rec.data[0],"Read Cmd : %02d , Value: %02d ", cmd, context.reg[context.reg_address]);
        enque(&rec);
       
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        context.reg_address_written = false;
       // sprintf(&rec.data[0],"On i2c_finish");
       // enque(&rec);


            
            

        break;
    default:
        break;
    }
}

// I2C Address is defined by 2 external pins on the IO + 0x20
uint8_t  read_i2c_adress() {
    uint8_t io0,io1,I2C_address;
   

    gpio_set_function(I2C_SLAVE_ADDRESS_IO0, GPIO_FUNC_SIO); // Set mode to software IO Control
    gpio_set_dir(I2C_SLAVE_ADDRESS_IO0, false); // Set IO to input
    gpio_pull_up(I2C_SLAVE_ADDRESS_IO0);  // Set to pull-up

    gpio_set_function(I2C_SLAVE_ADDRESS_IO1, GPIO_FUNC_SIO); // Set mode to software IO Control
    gpio_set_dir(I2C_SLAVE_ADDRESS_IO1, false); // Set IO to input
    gpio_pull_up(I2C_SLAVE_ADDRESS_IO1);  // Set to pull-up
        
    io0 = gpio_get(I2C_SLAVE_ADDRESS_IO0);
    io1 = gpio_get(I2C_SLAVE_ADDRESS_IO1);
    I2C_address = I2C_OFFSET_ADDRESS + (io1 << 1) + io0;
    return I2C_address;
}


static void  setup_i2c_slave(uint8_t i2c_add) {

    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(i2c1, I2C_BAUDRATE);
    
    // configure I2C1 for slave mode
    i2c_slave_init(i2c1, i2c_add, &i2c_slave_handler);
   //i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
  
}


static void setup_master() {
    gpio_init(I2C_MASTER_SDA_PIN);
    gpio_set_function(I2C_MASTER_SDA_PIN, GPIO_FUNC_I2C);
    // pull-ups are already active on slave side, this is just a fail-safe in case the wiring is faulty
    gpio_pull_up(I2C_MASTER_SDA_PIN);

    gpio_init(I2C_MASTER_SCL_PIN);
    gpio_set_function(I2C_MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_MASTER_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);
}

void send_master(uint8_t cmd, uint16_t wdata) {

    // Writing to A register
    int count;
    uint8_t buf[3];
    int buflgth;   // contains size of the buffer

    buflgth = 2; 
    buf[0] = cmd;       // command      
    buf[1] = wdata;    // gpio



        count = i2c_write_blocking(i2c0, context.i2c_add, buf, buflgth, false);
        if (count < 0) {
            puts("Couldn't write Register to slave");
            return;
        }
        printf("MAS: Write at register 0x%02d: %02d\n", buf[0], buf[1]);


        uint8_t ird[2];
        i2c_write_blocking(i2c0, context.i2c_add, buf, 1, false);
        i2c_read_blocking(i2c0, context.i2c_add, ird, buflgth-1, false);

        
        printf("MAS:Read Register 0x%02d = %d \r\n", cmd,ird[0]);
        
          
}


 

int main() {

    MESSAGE rec;
    uint16_t ctr;  // counter used for flashing led
    uint16_t pulse;  // limit for flashing led frequency
  
    status.all_flags =0 ;
    pulse = 200;    // slow led flashing frequency
    

    if (watchdog_caused_reboot()) { 
      status.watch = 1;
      pulse = 50;  // fast flashing led to indicate watchdog trig

    }
    
    gpio_init_mask(GPIO_BOOT_MASK); // set which lines will be GPIO 
    init_queue(); // initialise queue for serial message
    stdio_init_all();

    fprintf(stdout,"Selftest Version: %d.%d\n", IO_SELFTEST_VERSION_MAJOR, IO_SELFTEST_VERSION_MINOR);
  
    context.i2c_add  = read_i2c_adress();  // Setup I2C Address
    
    sprintf(&rec.data[0],"Pico Selftest boot for I2C address 0x%02x",context.i2c_add);
    enque(&rec); // Add message to the queue

    gpio_set_dir_masked(GPIO_SET_DIR_MASK,GPIO_SELF_DIR_MASK);
    gpio_put_masked(GPIO_SET_DIR_MASK,GPIO_SELF_OUT_MASK);


    setup_i2c_slave(context.i2c_add);
    set_default_serial();  // set default value for uart

    #ifdef DEBUG 
      setup_master();  // for development only, using loopback on jtag port
    #endif
 

    //enable_uart();
    //setup_spi_slave();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);  // Configure Pico led board
    gpio_put(PICO_DEFAULT_LED_PIN, 1);   // turn ON green led on Pico

    //watchdog_enable(500, 1); // enable the watchdog
    ctr = 0;
    int mess=0;

  while (1) {  // infinite loop, waiting for I2C command from Master

    watchdog_update();
    sleep_ms(10);
    ctr++;
    mess++;


    if (ctr > pulse) {
      gpio_put(PICO_DEFAULT_LED_PIN, 0); // Turn OFF board led
      sleep_ms(200);
      gpio_put(PICO_DEFAULT_LED_PIN, 1); // Turn ON board led
      ctr = 0;
    }


   if (mess > 1500) { // was 1500
      //printf("i2c add: 0x%02x\n", context.i2c_add); // for debug only
      fprintf(stdout,"Heartbeat I2C Slave add: 0x%02x  version: %d.%d\n", context.i2c_add, IO_SELFTEST_VERSION_MAJOR, IO_SELFTEST_VERSION_MINOR);
      mess = 0;
   }
    
  #ifdef DEBUG_UART 
   if (ctr >= pulse) {
    fprintf(stdout,"\n\n Test of uart command\n");
    test_serial_command(); 
  }
  #endif

  
   

    while (deque(&rec)) {
      gpio_put(PICO_DEFAULT_LED_PIN, 0); // Turn OFF board led
      printf("Pico %02x: %s\n", context.i2c_add,&rec.data[0]);  // send message to serial port
      sleep_ms(50);
      gpio_put(PICO_DEFAULT_LED_PIN, 1); // Turn ON board led
    }
  }

}
