cmake_minimum_required(VERSION 2.4.6)

project(segwayrmp)

OPTION(SEGWAYRMP_BUILD_TESTS "Build all of the SegwayRMP tests." OFF)
OPTION(SEGWAYRMP_BUILD_EXAMPLES "Build all of the SegwayRMP examples." OFF)

set(BUILD_SHARED_LIBS OFF)

SET(CMAKE_BUILD_TYPE "RELWITHDEBINFO")

# set the default path for built executables to the "bin" directory
IF(NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
ENDIF(NOT EXECUTABLE_OUTPUT_PATH)
# set the default path for built libraries to the "lib" directory
IF(NOT LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
ENDIF(NOT LIBRARY_OUTPUT_PATH)

# add the include folder to the include path
include_directories(${PROJECT_SOURCE_DIR}/include)

# Find the Serial library
IF(NOT serial_FOUND)
    find_package(serial REQUIRED)
ENDIF(NOT serial_FOUND)

# Check that serial was found
IF(NOT serial_FOUND)
    message(FATAL_ERROR "Serial library not found or provided, failing.")
ENDIF(NOT serial_FOUND)

include_directories(${serial_INCLUDE_DIRS})

# Find Boost
find_package(Boost COMPONENTS system filesystem thread REQUIRED)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

# Compile the libsegway Library
add_library(segwayrmp src/segwayrmp.cpp include/segwayrmp.h src/rmp_io.cpp include/rmp_io.h)
target_link_libraries(segwayrmp ${serial_LIBRARIES}
                                ${Boost_SYSTEM_LIBRARY}
                                ${Boost_FILESYSTEM_LIBRARY}
                                ${Boost_THREAD_LIBRARY})

IF(SEGWAYRMP_BUILD_EXAMPLES)
# Compile the Example program
add_executable(segwayrmp_example examples/segwayrmp_example.cpp)
# Link the Test program to the Serial library
target_link_libraries(segwayrmp_example segwayrmp)
ENDIF(SEGWAYRMP_BUILD_EXAMPLES)

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

INSTALL(TARGETS segwayrmp
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

INSTALL(FILES include/segwayrmp.h include/rmp_io.h DESTINATION include)

IF(NOT CMAKE_FIND_INSTALL_PATH)
    set(CMAKE_FIND_INSTALL_PATH ${CMAKE_ROOT})
ENDIF(NOT CMAKE_FIND_INSTALL_PATH)

# INSTALL(FILES Findsegwayrmp.cmake DESTINATION ${CMAKE_FIND_INSTALL_PATH}/Modules/)

ADD_CUSTOM_TARGET (uninstall @echo uninstall package)

IF (UNIX)
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
