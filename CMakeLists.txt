# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Asoboo C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()


# Add executable. Default name is the project name, version 0.1
add_executable(Asoboo 
    main.c
)

pico_set_program_name(Asoboo "Asoboo")
pico_set_program_version(Asoboo "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Asoboo 0)
pico_enable_stdio_usb(Asoboo 1)

# Add the standard library to the build
target_link_libraries(Asoboo
        pico_stdlib
        hardware_spi)

# Add the standard include files to the build
target_include_directories(Asoboo PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        "include/"
)


pico_add_extra_outputs(Asoboo)




# FatFS LIB :
target_sources(Asoboo PRIVATE
    lib/FatFs/src/ff.c
    lib/FatFs/src/diskio.c
    lib/FatFs/src/ffunicode.c
)

target_include_directories(Asoboo PRIVATE
    lib/FatFS/src
)