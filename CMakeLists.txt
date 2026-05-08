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

set(USER_SOURCES
    Core/Src/ringer.c
    Core/Src/tone.c
    Core/Src/easteregg.c
    Core/Src/easteregg_samples.c
    Core/Src/rotary.c
    Core/Src/uart_debug.c
    Core/Src/bt_hfp.c
    Core/Src/phone_fsm.c
)

set(USER_HEADERS
    Core/Inc/ringer.h
    Core/Inc/tone.h
    Core/Inc/easteregg.h
    Core/Inc/easteregg_samples.h
    Core/Inc/rotary.h
    Core/Inc/uart_debug.h
    Core/Inc/bt_hfp.h
    Core/Inc/phone_fsm.h
)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME femap_firmware)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${USER_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
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

    # Add user defined libraries
)

# Find the st-flash executable
find_program(ST_FLASH_EXE st-flash)

if(ST_FLASH_EXE)
  message(STATUS "Found st-flash at ${ST_FLASH_EXE}")
  add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
            ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generating .bin from ELF"
  )
  add_custom_target(flash
    COMMAND ${ST_FLASH_EXE} --connect-under-reset write
            ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin 0x08000000
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMENT "Flashing ${CMAKE_PROJECT_NAME}.bin to STM32..."
  )
  message(STATUS "Added 'flash' target")
else()
  message(WARNING "st-flash not found - flash target unavailable")
endif()

# Find the clang-format executable
find_program(CLANG_FORMAT_EXE clang-format)

# Check if it was found
if(CLANG_FORMAT_EXE)
  message(STATUS "Found clang-format at ${CLANG_FORMAT_EXE}")
else()
  message(WARNING "clang-format not found")
endif()

if(CLANG_FORMAT_EXE)
  add_custom_target(format
    COMMENT "Formatting files with clang-format..."
    COMMAND ${CLANG_FORMAT_EXE} -i ${USER_SOURCES} ${USER_HEADERS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
  message(STATUS "Added 'format' target")
else()
  message(WARNING
    "clang-format not found - format target unavailable")
endif()
