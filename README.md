# Selftest code firmware 700-2010-011

This software code is written to be loaded on Selftest board  500-1010-xxx.
The communication with the Pico controller use the I2C protocol configured as slave.

Command has received from the master controller ans the Pico Selftest firmware execute
the instruction and return answer to the sender

The hardware support for the firmware is on github location: https://github.com/FirstTestStation/Selftest_Board


## License
This project is licensed under the BSD 3-Clause License. See the [LICENSE](./LICENSE) file for more details.


## Project setup

This project was initially developed in 2020 on a Raspberry Pi 4, following the installation instructions from Getting Started with Pico_C.pdf Version 1.4.

The current version has been developed on a Raspberry Pi 5 using Visual Studio with the Raspberry Pi Pico extension, following the instructions in Getting Started with Pico_C.pdf dated 15 October 2024.

Based on https://github.com/vmilea/pico_i2c_slave. The pico_i2c_slave software has been added to enable the Raspberry Pi Pico to function as an I2C device.

For debugging, we utilize the GPIO pins of the Raspberry Pi 5, instead of the suggested debug probe.

Doxygen installation is required (sudo apt install doxygen).
Graphviz is required with doxygen (sudo apt install graphviz)


The compilation is performed using Visual Studio and the extension installed are:

* Raspberry Pi Pico Visual Studio Code extension 0.17.2
* Doxygen Documentation Generator v1.4.0
* Doxygen runner v1.8.0


## Building

Build of this cmake project is performed with Visual Studio using Pico Code extension (Compile Project)

## Development

* [`selftest.c`](IO_selftest/selftest.c) is the main source file for the firmware.
* [`CMakeLists.txt`](CMakeLists.txt) contains build instructions for CMake.
* [`pico_sdk_import.cmake`](pico_sdk_import.cmake) was (as usual) copied verbatim from the Pico SDK and allows CMake to interact with the SDK’s functionality.


## Firmware loading Instructions

### 1. Download the Latest Firmware UF2 File:
   You can download the latest firmware UF2 file from the following link:

   [Download UF2 File](https://github.com/FirstTestStation/Selftest_code_Firmware/blob/main/build/IO_selftest/SELFTEST_CODE.uf2)

   On the GitHub page, select **Download Raw file** to get the UF2 file.

### 2. Prepare your Raspberry Pi Pico:
   - Make sure your Raspberry Pi Pico is not connected to your computer.

### 3. Enter Bootloader Mode:
   - Hold down the **BOOTSEL** button on your Raspberry Pi Pico.
   - While holding the **BOOTSEL** button, connect the Pico to your computer via USB.
   - Release the **BOOTSEL** button after the Pico appears as a mass storage device on your computer.

### 4. Copy the UF2 File:
   - Once in bootloader mode, the Raspberry Pi Pico will show up as a removable storage drive named `RPI-RP2`.
   - Copy the downloaded UF2 file (e.g., `SELFTEST_CODE.uf2`) to the `RPI-RP2` drive.

### 5. Eject the Device:
   - After the file is copied, safely eject the `RPI-RP2` drive from your computer.

### 6. Reboot:
   - The Raspberry Pi Pico will automatically reboot and start running the new firmware.


## Installation


* When software loaded, the Pico board should be installed on the location marked ST_CTRL on Selftest Board.
* On board Pico Led will flash slowly (heartbeat) on power ON.


### Setup

Follow the instructions in [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf) to setup your build environment.


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
| 30 | Set GPx strength = 2mA  | Set GPx output max current |
| 31 | Set GPx strength = 4mA  | Set GPx output max current |
| 32 | Set GPx strength = 8mA  | Set GPx output max current |
| 33 | Set GPx strength = 12mA | Set GPx output max current |
| 35 | Get Gpx drive strength  | Read strength:  0: 2mA, 1: 4mA, 2: 8mA, 3: 12mA |
| 41 | Set pull_up GPx         | Add pull-up to Gpx  |
| 45 | Get pull-up GPx         | Read pull-up state (1: pull-up active) |  
| 50 | Disable pulls           | Remove  pull-up and pull-down  |
| 51 | Set pull_down GPx       | Add pull-down to Gpx    |  
| 55 | Get pull-down GPx       | Read pull-down state (1: pull-down active) |  
| 60 | Set Pads State value    | Set Pads State register for use later  |
| 61 | Set GPx to Pads State   | Write contains of command 60 on GPx |   
| 65 | Get Pads state Gpx      | Read PAD register for Gpx |
| 75 | Get GPIO function       | Read gpio function associated to gpio number|
|    |                         | XIP = 0, SPI=1, UART=2, I2C =3, PWM=4, SIO = 5 | 
|    |                         | PIO0 =6, PIO1=7, GPCK=8, USB=9, NULL = 0x1f |
| 80 | Set PWM State           | 1: Enable   0: Disable |   
| 81 | Set PWM Frequency       | Freq in Khz 0=100Hz, 1= 1Khz, 255 = 255KHz (8 bits)| 
|    |                         |                                           |
| 100| Device Status           | Bit Status  (8 bits)             |  
|    | Bit 0                   | Config Completed   0: true |
|    | Bit 1                   | Command accepted   0: true |
|    | Bit 2                   | Error  1= true |
|    | Bit 3                   | watchdog triggered 1= true|
| 101| Enable  UART            | setup UART mode  0: TX/RX, 1: TX/RX + CTS/RTS  |
| 102| Disable UART            | setup UART to SIO mode:  0:input gpio, 1:output gpio  |
| 103| Set UART protocol       | set UART protocol, see bits definition below |
| 105| Get UART config         | return 1 byte config protocol:  |
|    |                         | Bit 7-6  Baudrate   0:19200, 1:38400,2: 57600, 3:115200 |
|    |                         | Bit 5-4  parity  0: None, 1: Even, 2: Odd  |
|    |                         | Bit 3:2  data bits  0:5, 1:6, 2:7,3:8  |
|    |                         | Bit 1    stop bits  0:1 stop , 1:2 stops |
|    |                         | Bit 0    handshake RTS/CTS    0: None, 1: Used |
|    |                         |          |
| 111| Enable  SPI             | 0: Enable, 1:Enable, Default configuration |
| 112| Disable SPI             | Setup SPI to SIO mode:  0:input gpio, 1:output gpio  |
| 113| Set SPI protocol        | set SPI protocol, see bits definition below |
| 115| Get SPI config          | return 1 byte config protocol:  |
|    |                         | Bit 7:4  Baudrate  Value * 100Khz  
|    |                         | Bit 3  Databits  0:8, 1:16  |
|    |                         | Bit 2:1  Mode  Clock Polarity, Clock Phase |
|    |                         | ___ Mode 0:  Cpol:0 , Cpha 0 |
|    |                         | ___ Mode 1:  Cpol:0 , Cpha 1 |
|    |                         | ___ Mode 2:  Cpol:1 , Cpha 0 |
|    |                         | ___ Mode 3:  Cpol:1 , Cpha 1 |
|    |                         | Bit 0  SPI Status, 0:disable, 1:enable  Read only|
