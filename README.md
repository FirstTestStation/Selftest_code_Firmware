# I2C slave library for the Raspberry Pi Pico

Modified to run for my application 

The Raspberry Pi Pico C/C++ SDK has all you need to write an I2C master, but is curiously lacking when it comes to I2C in slave mode. This library fills that gap to easily turn the Pico into an I2C slave.

## Examples
 

### Setup

Follow the instructions in [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf) to setup your build environment.


## Links


## Original Authors

Valentin Milea <valentin.milea@gmail.com>


## Adaptation Authors
Daniel Lockhead <daniellockhead@gmail.com>


## I2C Command supported

Pico i2C Slave mode Protocol

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



## 8 Bit I/O port I2C Command

| Command_Byte | Function   |  Description |
| --- | --- | --- |
|80  | Set IO Mask Port 0    | 8 bit mask direction   0 = In , 1 = Out |
|81  | Set IO Output Port 0  | Set Output Line   0= Low  1=High       |
|85  | Read IO Input Port 0  | Get Input Line    0= Low  1=High      |
|90  | Set IO Mask Port 1    | 8 bit mask direction   0 = In , 1 = Out |
|91  | Set IO Output Port 1  | Set Output Line   0= Low  1=High     |
|95  | Read IO Input Port 1  | Get Input Line    0= Low  1=High        |
|100 | Device Status         | Bit Status  (8 bits)             |  
|    | Bit 0                 | Config Completed   0: true |
|    | Bit 1                 | Command accepted   0: true |
|    | Bit 2                 | Error  1= true|
|    | Bit 3                 | watchdog trigged 1= true|
