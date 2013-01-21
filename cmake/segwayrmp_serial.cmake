# Find Serial if support requested
if(SEGWAYRMP_USE_SERIAL)
  find_package(serial QUIET)

  set(SEGWAYRMP_USE_SERIAL FALSE)
  if(serial_FOUND)
    set(SEGWAYRMP_USE_SERIAL TRUE)
    include_directories(${serial_INCLUDE_DIRS})
    list(APPEND SEGWAYRMP_LINK_LIBS ${serial_LIBRARIES})
  else()
    # Could not find serial either through find_package or ROS
    message("--")
    message("-- Serial support disabled: Serial library not found.")
    message("--")
  endif()

  if(SEGWAYRMP_USE_SERIAL)
    message("-- Building SegwayRMP with serial support")
    list(APPEND SEGWAYRMP_SRCS src/impl/rmp_serial.cc)
    add_definitions(-DSEGWAYRMP_USE_SERIAL)
  endif()
endif()
