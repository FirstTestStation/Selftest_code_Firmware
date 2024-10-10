

   set (IO_SELFTEST_VERSION_MAJOR 1)
   set (IO_SELFTEST_VERSION_MINOR 0)


   message(STATUS ">>>DIRECTORY USED")
   message(STATUS "Source= ${PROJECT_SOURCE_DIR}")
   message(STATUS "Binary=${PROJECT_BINARY_DIR}")


   configure_file (
   "${PROJECT_SOURCE_DIR}/IO_selftest/include/userconfig.h.in"
   "${PROJECT_BINARY_DIR}/IO_selftest/include/userconfig.h"  ) 
   include_directories("${PROJECT_BINARY_DIR}/IO_selftest/include") 

   add_library(serial INTERFACE) #DL
   target_include_directories(serial INTERFACE ./include)
   target_sources(serial INTERFACE serial.c)

   add_library(spi_slave INTERFACE) #DL
  target_include_directories(spi_slave INTERFACE ./include)
  target_sources(spi_slave INTERFACE spi_slave.c)


   add_executable(selftest selftest.c serial.c spi_slave.c)
 #add_executable(selftest selftest.c)

  pico_enable_stdio_uart(selftest 0)
   pico_enable_stdio_usb(selftest 1)

   pico_add_extra_outputs(selftest)

   target_compile_options(selftest PRIVATE -Wall)

   target_link_libraries(selftest
    i2c_slave
    pico_stdlib
    hardware_spi
    hardware_pwm
    )
   #target_link_libraries(selftest pico_stdlib hardware_spi)
   
