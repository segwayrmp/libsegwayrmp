# Find Serial if support requested
if(SEGWAYRMP_USE_SERIAL)
  find_package(serial QUIET)

  set(SEGWAYRMP_USE_SERIAL FALSE)
  if(serial_FOUND)
    set(SEGWAYRMP_USE_SERIAL TRUE)
    include_directories(serial_INCLUDE_DIRS)
    list(APPEND SEGWAYRMP_LINK_LIBS ${serial_LIBRARIES})
  else(serial_FOUND)
    # Try to use ROS to find it
    set(ROS_ROOT $ENV{ROS_ROOT})
    if(DEFINED ROS_ROOT)
      message("-- Using ROS to resolve serial dependency")
      # Setup ROS build
      include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)
      rosbuild_init()
      # Add include and link directories
      include_directories(${${PROJECT_NAME}_INCLUDE_DIRS})
      link_directories(${${PROJECT_NAME}_LIBRARY_DIRS})
      # Add ROS libraries for this "project"
      list(APPEND SEGWAYRMP_LINK_LIBS ${${PROJECT_NAME}_LIBRARIES})
      # Set the default path for built executables to the "bin" directory
      set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
      # set the default path for built libraries to the "lib" directory
      set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
    else(DEFINED ROS_ROOT)
      # Could not find serial either through find_package or ROS
      message("--")
      message("-- Serial support disabled: Serial library not found.")
      message("--")
    endif(DEFINED ROS_ROOT)
  endif(serial_FOUND)

  if(SEGWAYRMP_USE_SERIAL)
    message("-- Building with serial support")
    list(APPEND SEGWAYRMP_SRCS src/impl/rmp_serial.cc)
    add_definitions(-DSEGWAYRMP_USE_SERIAL)
  endif(SEGWAYRMP_USE_SERIAL)
endif(SEGWAYRMP_USE_SERIAL)
