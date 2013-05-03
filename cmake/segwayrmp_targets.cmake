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

install(
  FILES       cmake/Findsegwayrmp.cmake
  DESTINATION share/libsegwayrmp/
)

install(
  FILES package.xml
  DESTINATION share/libsegwayrmp/
)

# For now, enable installing the FTDI library on linux
if(UNIX)
  # If linux
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(
      FILES lib/libftd2xx.a
      DESTINATION lib/
    )
  endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
endif(UNIX)

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
