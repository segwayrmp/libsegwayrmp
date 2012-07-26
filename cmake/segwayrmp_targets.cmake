## This file setups the targets like install and uninstall

# Configure make install
if (NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX /usr/local)
endif(NOT CMAKE_INSTALL_PREFIX)

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

if (NOT CMAKE_FIND_INSTALL_PATH)
  set(CMAKE_FIND_INSTALL_PATH ${CMAKE_ROOT})
endif(NOT CMAKE_FIND_INSTALL_PATH)

install(
  FILES       cmake/Findsegwayrmp.cmake
  DESTINATION ${CMAKE_FIND_INSTALL_PATH}/Modules/
)

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
