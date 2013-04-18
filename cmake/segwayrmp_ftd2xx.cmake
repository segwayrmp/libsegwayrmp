# This files configures the library to build against one of the provided
# ftd2xx libraries shipped with this library.

include_directories(${PROJECT_SOURCE_DIR}/ftd2xx/include)

list(APPEND SEGWAYRMP_SRCS src/impl/rmp_ftd2xx.cc)

set(bitness x32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(bitness x64)  
endif()

# For Unix like systems
if(UNIX)
  # If Linux
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    list(APPEND SEGWAYRMP_LINK_LIBS
      ${PROJECT_SOURCE_DIR}/ftd2xx/linux/${bitness}/libftd2xx.a
      dl
      rt
      pthread
    )
    # Copy this file for rosmake to find it
    file(
      COPY ${PROJECT_SOURCE_DIR}/ftd2xx/linux/${bitness}/libftd2xx.a
      DESTINATION ${PROJECT_SOURCE_DIR}/lib/
    )
  endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # If Mac OS X
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message("")
    message("-- **********************************************************")
    message("-- On OS X the ftd2xx library needs to be installed before")
    message("-- running applications:")
    message("-- http://www.ftdichip.com/Drivers/D2XX/MacOSX/D2XX1.1.12.dmg")
    message("-- **********************************************************")
    message("")
    list(APPEND SEGWAYRMP_LINK_LIBS
      ${PROJECT_SOURCE_DIR}/ftd2xx/osx/libftd2xx.1.1.12.dylib
    )
  endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
endif(UNIX)

# For Windows
if(WIN32)
  list(APPEND SEGWAYRMP_LINK_LIBS
    ${PROJECT_SOURCE_DIR}/ftd2xx/win/${bitness}/ftd2xx.lib
  )
endif(WIN32)
