## Project Setup
cmake_minimum_required(VERSION 2.6)

project(segwayrmp)
set (segwayrmp_VERSION_MAJOR 0)
set (segwayrmp_VERSION_MINOR 2)
set (segwayrmp_VERSION_PATCH 8)
set (segwayrmp_VERSION
  "${segwayrmp_VERSION_MAJOR}.${segwayrmp_VERSION_MINOR}.${segwayrmp_VERSION_PATCH}")

## Configurations

include(cmake/segwayrmp_settings.cmake)

## Configure the build system

# Add the include folder to the include path
include_directories(${PROJECT_SOURCE_DIR}/include)

# Find boost
find_package(Boost COMPONENTS system thread REQUIRED)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

# Set the source files, headers, and link libraries
set(SEGWAYRMP_SRCS src/segwayrmp.cc src/impl/rmp_io.cc)
set(SEGWAYRMP_HEADERS include/segwayrmp/segwayrmp.h)
set(SEGWAYRMP_LINK_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

# Configure Serial support
include(cmake/segwayrmp_serial.cmake)

# Configure FTD2XX support
include(cmake/segwayrmp_ftd2xx.cmake)

# Configure Graphical User Interface
include(cmake/segwayrmp_gui.cmake)

## Build the segwayrmp project

# Compile the Library
add_library(segwayrmp ${SEGWAYRMP_SRCS})
target_link_libraries(segwayrmp ${SEGWAYRMP_LINK_LIBS})

## Build Examples

set(SEGWAYRMP_EXAMPLE_SRCS examples/segwayrmp_example.cc)
set(SEGWAYRMP_EXAMPLE_LINK_LIBS segwayrmp)
include(cmake/segwayrmp_examples.cmake)

## Build Tests

set(SEGWAYRMP_TEST_SRCS tests/segwayrmp_tests.cc)
set(SEGWAYRMP_TEST_LINK_LIBS segwayrmp)
include(cmake/segwayrmp_tests.cmake)

## Setup Install/Uninstall Targets

include(cmake/segwayrmp_targets.cmake)
