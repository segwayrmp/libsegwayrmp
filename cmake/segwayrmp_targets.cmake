## This file setups the targets like install and uninstall

install(
  TARGETS segwayrmp
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(
  FILES       include/segwayrmp/segwayrmp.h
  DESTINATION include/segwayrmp
)

configure_file(
  "cmake/libsegwayrmpConfig.cmake.in"
  "cmake/libsegwayrmpConfig.cmake"
  @ONLY
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/libsegwayrmpConfig.cmake 
  DESTINATION share/libsegwayrmp
)

install(
  FILES package.xml
  DESTINATION share/libsegwayrmp/
)

# For now, enable installing the FTDI library on linux
if(UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  install(
    FILES lib/libftd2xx.a
    DESTINATION lib/
  )
endif(UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Enable pkg-configuration file generation for linux
if(UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  configure_file ("cmake/libsegwayrmp.pc.in" "cmake/libsegwayrmp.pc" @ONLY)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/libsegwayrmp.pc 
    DESTINATION lib/pkgconfig/
  )
endif(UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Configure make uninstall
add_custom_target(uninstall @echo uninstall package)

if (UNIX)
  add_custom_command(
    COMMENT "uninstall package"
    COMMAND xargs ARGS rm < install_manifest.txt
    TARGET  uninstall
  )
else(UNIX)
  add_custom_command(
    COMMENT "uninstall only implemented in unix"
    TARGET  uninstall
  )
endif(UNIX)
