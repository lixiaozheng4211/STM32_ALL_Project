cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME U8G2_)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        Core/u8g2/mui.c
        Core/u8g2/u8g2.h
        Core/u8g2/u8x8.h
        Core/u8g2/u8log.c
        Core/u8g2/mui_u8g2.c
        Core/u8g2/u8g2_arc.c
        Core/u8g2/u8g2_box.c
        Core/u8g2/u8x8_8x8.c
        Core/u8g2/u8x8_cad.c
        Core/u8g2/u8g2_font.c
        Core/u8g2/u8g2_line.c
        Core/u8g2/u8x8_byte.c
        Core/u8g2/u8x8_gpio.c
        Core/u8g2/u8g2_fonts.c
        Core/u8g2/u8g2_setup.c
        Core/u8g2/u8log_u8g2.c
        Core/u8g2/u8log_u8x8.c
        Core/u8g2/u8x8_fonts.c
        Core/u8g2/u8x8_setup.c
        Core/u8g2/u8x8_u8toa.c
        Core/u8g2/u8g2_bitmap.c
        Core/u8g2/u8g2_buffer.c
        Core/u8g2/u8g2_button.c
        Core/u8g2/u8g2_circle.c
        Core/u8g2/u8g2_hvline.c
        Core/u8g2/u8x8_string.c
        Core/u8g2/u8x8_u16toa.c
        Core/u8g2/u8g2_d_setup.c
        Core/u8g2/u8g2_d_memory.c
        Core/u8g2/u8g2_cleardisplay.c
        Core/u8g2/u8g2_input_value.c
        Core/u8g2/u8g2_intersection.c
        Core/u8g2/u8g2_kerning.c
        Core/u8g2/u8g2_ll_hvline.c
        Core/u8g2/u8g2_message.c
        Core/u8g2/u8g2_polygon.c
        Core/u8g2/u8g2_selection_list.c
        Core/u8g2/u8x8_capture.c
        Core/u8g2/u8x8_d_ssd1315_128x64_noname.c
        Core/u8g2/u8x8_debounce.c
        Core/u8g2/u8x8_display.c
        Core/u8g2/u8x8_input_value.c
        Core/u8g2/u8x8_message.c
        Core/u8g2/u8x8_selection_list.c)


# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
        Core/u8g2/
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        Core/u8g2
        # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    Core/u8g2/
    # Add user defined libraries
)
