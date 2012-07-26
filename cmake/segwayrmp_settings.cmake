## This cmake file contains different settings for the project

# Should the examples be built?
option(SEGWAYRMP_BUILD_EXAMPLES "Build the examples?" ON)

# Should the tests be built?
option(SEGWAYRMP_BUILD_TESTS "Build the tests?" OFF)

# Should support for control via Serial be built?
option(SEGWAYRMP_USE_SERIAL "Build with Serial (RS-232) Support?" ON)

# Set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# Uncomment the line below to set the build type
set(CMAKE_BUILD_TYPE "RELWITHDEBINFO")
