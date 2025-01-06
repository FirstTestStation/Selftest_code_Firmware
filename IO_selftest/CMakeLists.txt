

   set (IO_SELFTEST_VERSION_MAJOR 1)
   set (IO_SELFTEST_VERSION_MINOR 1)


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


   add_executable(${PROJECT_NAME} selftest.c serial.c spi_slave.c)
 #add_executable(selftest selftest.c)

  pico_enable_stdio_uart(${PROJECT_NAME} 0)
   pico_enable_stdio_usb(${PROJECT_NAME} 1)

   pico_add_extra_outputs(${PROJECT_NAME})

   # target_compile_options(${PROJECT_NAME} PRIVATE -Wall)

   target_link_libraries(${PROJECT_NAME}
    i2c_slave
    pico_stdlib
    hardware_spi
    hardware_pwm
    )
   
   
# copy files to location requested by pico app
set(UF2_FILE ${CMAKE_BINARY_DIR}/IO_selftest/${PROJECT_NAME}.uf2)
set(ELF_FILE ${CMAKE_BINARY_DIR}/IO_selftest/${PROJECT_NAME}.elf)


# Copy the .uf2 file
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${UF2_FILE} ${CMAKE_BINARY_DIR}
	COMMENT "Copying .uf2 file to ${CMAKE_BINARY_DIR}"
)

# Copy the .elf file
add_custom_command(
    TARGET ${PROJECT_NAME}  POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${ELF_FILE} ${CMAKE_BINARY_DIR}
	COMMENT "Copying .elf file to ${CMAKE_BINARY_DIR}"
)

