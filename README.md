# Selftest board firmware 

This software code is written to be loaded on selftest board  500-1010-020.
The communication with the Pico controller use the I2C protocol configured as slave.

Command has received from the master controller ans the Pico selftest firmware execute
the instruction and return answer to the sender

 

### Setup

Follow the instructions in [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf) to setup your build environment.


## Links

## Authors
Daniel Lockhead <daniellockhead@gmail.com>


## Contributor Authors
Thanks to 
    Valentin Milea <valentin.milea@gmail.com>
to provide I2C_Slave library 


## I2C Command supported

Pico i2C Selftest Protocol

I2C Command is 2 bytes long:  Command_byte (1 byte) + Data (1 byte)

| Command_Byte | Function   |  Description |
| --- | --- | --- |
| 00 |  Reserved  | used for special purpose |
| 01 | Get Major Version  | return major version of firmware |
| 02 | Get Minor Version  | return minor version of firmware |
| 10 | Clear GPx          | Write 0 on GPx                   |
| 11 | Set GPx            | Write 1 on GPx                   |
| 15 | Read GPx           | Read GPx state    | 
| 20 | Set Dir GPx Out    | Set direction Out for Gpx  |                                   
| 21 | Set Dir GPx In     | Set direction In for Gpx  |                                   
| 25 | Get Dir GPx        | Read GPX direction, 0 = In , 1 = Out |
| 30 | Set GPx strenght = 2mA  | Set GPx output max current |
| 31 | Set GPx strenght = 4mA  | Set GPx output max current |
| 32 | Set GPx strenght = 8mA  | Set GPx output max current |
| 33 | Set GPx strenght = 12mA | Set GPx output max current |
| 35 | Get Gpx drive strenght  | Read strenght:  0: 2mA, 1: 4mA, 2: 8mA, 3: 12mA |
| 41 | Set pull_up GPx         | Add pull-up to Gpx  |
| 45 | Get pull-up GPx         | Read pull-up state (1: pull-up active) |  
| 50 | Disable pulls           | Remove  pull-up and pull-down  |
| 51 | Set pull_down GPx       | Add pull-down to Gpx    |  
| 55 | Get pull-down GPx       | Read pull-down state (1: pull-down active) |  
| 60 | Set Pads State value    | Set Pads State register for use later  |
| 61 | Set GPx to Pads State   | Write contains of command 60 on GPx |   
| 65 | Get Pads state Gpx      | Read PAD register for Gpx |
| 75 | Get GPIO function       | Read gpio function associated to gpio number|
                               | XIP = 0, SPI=1, UART=2, I2C =3, PWM=4, SIO = 5 | 
                               | PIO0 =6, PIO1=7, GPCK=8, USB=9, NULL = 0x1f | 
|100 | Device Status           | Bit Status  (8 bits)             |  
|    | Bit 0                   | Config Completed   0: true |
|    | Bit 1                   | Command accepted   0: true |
|    | Bit 2                   | Error  1= true |
|    | Bit 3                   | watchdog trigged 1= true|
| 101| Enable  UART            | setup uart mode  0: TX/RX, 1: TX/RX + CTS/RTS  |
| 102| Disable UART            | setup uart to SIO mode:  0:input gpio, 1:output gpio  |
| 103| Set UART protocol       | set uart protocol, see bits definition below |
| 105| Get UART config         | return 1 byte config protocol:  |
                               | Bit 7-6  Baudrate   0:9600, 1:38400,2: 57600, 3:115200 |
                               | Bit 5-4  parity  0: None, 1: Even, 2: Odd  |
                               | Bit 3:2  data bits  0:5, 1:6, 2:7,3:8  |
                               | Bit 1    stop bits  0:1 stop , 1:2 stops |
                               | Bit 0    handshake RTS/CTS    0: None, 1: Used |


