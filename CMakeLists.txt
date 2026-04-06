



cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 14)
set(PICO_COPY_TO_RAM 1)

set(PICO_BOARD pico)  # uncomment to build for the original "Pico" with RP2040
#set(PICO_BOARD pico2)  # uncomment to build for the newer "Pico 2" with RP2350

include(pico_sdk_import.cmake)
project(Myztrion)
pico_sdk_init()

#add_compile_definitions(DPICO_PLATFORM=rp2040)  # uncomment to build for original Rpi Pico
add_compile_definitions(CFG_TUD_CDC_RX_BUFSIZE=1024)
add_compile_definitions(CFG_TUD_CDC_TX_BUFSIZE=1024)
add_compile_definitions(CFG_TUD_CDC_EP_BUFSIZE=1024)

# include_directories("include")
add_executable(Myztrion
        Myztrion.c
)


# file(GLOB SOURCES "include/*.c")

pico_enable_stdio_usb(Myztrion 1)
pico_enable_stdio_uart(Myztrion 0)

pico_add_extra_outputs(Myztrion)
target_link_libraries(Myztrion pico_stdlib hardware_pwm
        pico_unique_id hardware_watchdog hardware_adc hardware_i2c
        hardware_pio hardware_clocks hardware_spi pico_multicore
        hardware_dma hardware_irq )

#add_executable(pio_Myztrion)
