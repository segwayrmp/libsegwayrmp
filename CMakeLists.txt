## Project Setup
cmake_minimum_required(VERSION 2.4.6)

project(segwayrmp)

## Configurations

option(SEGWAYRMP_BUILD_TESTS "Build all of the SegwayRMP tests." OFF)
option(SEGWAYRMP_BUILD_EXAMPLES "Build all of the SegwayRMP examples." OFF)
option(SEGWAYRMP_USE_SERIAL "Build the library with serial support, requires http://github.com/wjwwood/serial/" ON)
option(SEGWAYRMP_USE_FTD2XX "Build the library with FTD2XX support, requires http://www.ftdichip.com/Drivers/D2XX.htm" ON)

# Check to make sure at least one implementation is on
IF(NOT SEGWAYRMP_USE_SERIAL AND NOT SEGWAYRMP_USE_FTD2XX)
    message(FATAL_ERROR "libsegwayrmp: You must select at least one implementation (Serial or FTD2XX).")
ENDIF(NOT SEGWAYRMP_USE_SERIAL AND NOT SEGWAYRMP_USE_FTD2XX)

# Allow for building shared libs override
IF(NOT BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS OFF)
ENDIF(NOT BUILD_SHARED_LIBS)

# Allow for build type override
IF(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RELWITHDEBINFO")
ENDIF(NOT CMAKE_BUILD_TYPE)

# set the default path for built executables to the "bin" directory
IF(NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
ENDIF(NOT EXECUTABLE_OUTPUT_PATH)
# set the default path for built libraries to the "lib" directory
IF(NOT LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
ENDIF(NOT LIBRARY_OUTPUT_PATH)

## Configure build system

set(SEGWAYRMP_IMPL_FOUND FALSE)

# Add the include folder to the include path
include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/include/impl)

# Add default source files
set(SEGWAYRMP_SRCS src/segwayrmp.cpp src/rmp_io.cpp)
# Add default header files
set(SEGWAYRMP_HEADERS include/segwayrmp.h include/rmp_io.h)

## Find and add Boost

find_package(Boost COMPONENTS system filesystem thread REQUIRED)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

set(SEGWAYRMP_LINK_LIBS ${Boost_SYSTEM_LIBRARY}
                        ${Boost_FILESYSTEM_LIBRARY}
                        ${Boost_THREAD_LIBRARY})

## Serial implementation setup

IF(SEGWAYRMP_USE_SERIAL)
    # Find the Serial library
    IF(NOT serial_FOUND)
        find_package(serial QUIET)
    ENDIF(NOT serial_FOUND)
    
    # Check that serial was found
    IF(NOT serial_FOUND)
        message("libsegwayrmp: Serial library not found, serial support will not be built.")
    ELSE(NOT serial_FOUND)
        set(SEGWAYRMP_IMPL_FOUND TRUE)
        message(STATUS "libsegwayrmp: Serial enabled and library found, building with serial support.")
    ENDIF(NOT serial_FOUND)
ENDIF(SEGWAYRMP_USE_SERIAL)
IF(serial_FOUND)
    include_directories(${serial_INCLUDE_DIRS})
    
    list(APPEND SEGWAYRMP_LINK_LIBS ${serial_LIBRARIES})
    
    list(APPEND SEGWAYRMP_SRCS src/impl/rmp_serial.cpp)
    
    list(APPEND SEGWAYRMP_HEADERS include/impl/rmp_serial.h)
    
    set(SEGWAYRMP_SERIAL_SUPPORT TRUE)
ELSE(FTD2XX_FOUND)
    set(SEGWAYRMP_SERIAL_SUPPORT FALSE)
ENDIF(serial_FOUND)

## FTD2XX implementation setup

IF(SEGWAYRMP_USE_FTD2XX)
    # Find the FTD2XX Headers
    find_path(FTD2XX_INCLUDE_DIR ftd2xx.h WinTypes.h /usr/include/ /usr/local/include/)
    # Find the FTD2XX Library
    find_library(FTD2XX_LIBRARIES NAMES ftd2xx PATHS /usr/lib /usr/local/lib)
    
    set(FTD2XX_FOUND TRUE)
    
    IF(NOT FTD2XX_INCLUDE_DIR)
        set(FTD2XX_FOUND FALSE)
    ENDIF(NOT FTD2XX_INCLUDE_DIR)
    
    IF(NOT FTD2XX_LIBRARIES)
        set(FTD2XX_FOUND FALSE)
    ENDIF(NOT FTD2XX_LIBRARIES)
    
    IF(NOT FTD2XX_FOUND)
        message("libsegwayrmp: FTD2XX library not found, FTD2XX usb support will not be built.")
    ELSE(NOT FTD2XX_FOUND)
        set(SEGWAYRMP_IMPL_FOUND TRUE)
        message(STATUS "libsegwayrmp: FTD2XX enabled and library found, building with FTD2XX usb support.")
    ENDIF(NOT FTD2XX_FOUND)
ENDIF(SEGWAYRMP_USE_FTD2XX)
IF(FTD2XX_FOUND)
    include_directories(${FTD2XX_INCLUDE_DIR})
    
    list(APPEND SEGWAYRMP_LINK_LIBS ${FTD2XX_LIBRARIES})
    
    list(APPEND SEGWAYRMP_SRCS src/impl/rmp_ftd2xx.cpp)
    
    list(APPEND SEGWAYRMP_HEADERS_ORIG include/impl/rmp_ftd2xx.h)
    
    set(SEGWAYRMP_FTD2XX_SUPPORT TRUE)
ELSE(FTD2XX_FOUND)
    set(SEGWAYRMP_FTD2XX_SUPPORT FALSE)
ENDIF(FTD2XX_FOUND)

## Perform header configuration

configure_file(${PROJECT_SOURCE_DIR}/include/segwayrmp.h.in ${PROJECT_SOURCE_DIR}/include/segwayrmp.h)

## Build the library

# Ensure at least one implementation is available
IF(NOT SEGWAYRMP_IMPL_FOUND)
    message(FATAL_ERROR "libsegwayrmp: No implementation can be built and so the library cannot communicate, failing.")
ENDIF(NOT SEGWAYRMP_IMPL_FOUND)

# Compile the libsegway Library
add_library(segwayrmp ${SEGWAYRMP_SRCS})
target_link_libraries(segwayrmp ${SEGWAYRMP_LINK_LIBS})

# Add C++ Preprocessor Flags
IF(SEGWAYRMP_USE_SERIAL AND serial_FOUND)
    set_target_properties(segwayrmp PROPERTIES COMPILE_DEFINITIONS SEGWAYRMP_SERIAL_SUPPORT)
ENDIF(SEGWAYRMP_USE_SERIAL AND serial_FOUND)

IF(SEGWAYRMP_USE_FTD2XX AND FTD2XX_FOUND)
    set_target_properties(segwayrmp PROPERTIES COMPILE_DEFINITIONS SEGWAYRMP_FTD2XX_SUPPORT)
ENDIF(SEGWAYRMP_USE_FTD2XX AND FTD2XX_FOUND)

## Build Examples

# If specified
IF(SEGWAYRMP_BUILD_EXAMPLES)
    # Compile the Example program
    add_executable(segwayrmp_example examples/segwayrmp_example.cpp)
    # Link the Test program to the Serial library
    target_link_libraries(segwayrmp_example segwayrmp)
ENDIF(SEGWAYRMP_BUILD_EXAMPLES)

## Build Tests

# If specified
IF(SEGWAYRMP_BUILD_TESTS)
    # Find Google Test                                                                                                                                                               
    enable_testing()
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
    
    # Compile the Test program
    add_executable(segwayrmp_tests test/segwayrmp_tests.cpp)
    # Link the Test program to the segwayrmp library
    target_link_libraries(segwayrmp_tests segwayrmp ${GTEST_BOTH_LIBRARIES})
    add_test(AllTestsIntest_serial segwayrmp_tests)
ENDIF(SEGWAYRMP_BUILD_TESTS)

# Configure make install
IF(NOT CMAKE_INSTALL_PREFIX)
    SET(CMAKE_INSTALL_PREFIX /usr/local)
ENDIF(NOT CMAKE_INSTALL_PREFIX)

## Install setup

# Install the library
IF(NOT SEGWAYRMP_DONT_SETUP_INSTALL)
    INSTALL(
            TARGETS segwayrmp
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
    )
    
    INSTALL(FILES ${SEGWAYRMP_HEADERS} DESTINATION include)
    
    IF(NOT CMAKE_FIND_INSTALL_PATH)
        set(CMAKE_FIND_INSTALL_PATH ${CMAKE_ROOT})
    ENDIF(NOT CMAKE_FIND_INSTALL_PATH)
    
    INSTALL(FILES Findsegwayrmp.cmake DESTINATION ${CMAKE_FIND_INSTALL_PATH}/Modules/)
    
    ADD_CUSTOM_TARGET(uninstall @echo uninstall package)
    
    IF(UNIX)
      ADD_CUSTOM_COMMAND(
                         COMMENT "uninstall package"
                         COMMAND xargs ARGS rm < install_manifest.txt
                         TARGET  uninstall
      )
    ELSE(UNIX)
      ADD_CUSTOM_COMMAND(
                         COMMENT "uninstall only implemented in unix"
                         TARGET  uninstall
      )
    ENDIF(UNIX)
ENDIF(NOT SEGWAYRMP_DONT_SETUP_INSTALL)