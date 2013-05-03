find_path(segwayrmp_INCLUDE_DIRS segwayrmp/segwayrmp.h /usr/include 
          /usr/local/include "$ENV{NAMER_ROOT}")

find_library(segwayrmp_LIBRARY segwayrmp /usr/lib /usr/local/lib 
  "$ENV{NAMER_ROOT}")
set(segwayrmp_LIBRARIES ${segwayrmp_LIBRARY})

# The linux version also installs ftd2xx which needs to be found
if(UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  find_library(ftd2xx_LIBRARY ftd2xx /usr/lib /usr/local/lib 
    "$ENV{NAMER_ROOT}")
  set(segwayrmp_LIBRARIES ${segwayrmp_LIBRARY} ${ftd2xx_LIBRARY})
endif(UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux") 

if(NOT segwayrmp_INCLUDE_DIRS OR NOT segwayrmp_LIBRARIES)
  set(segwayrmp_FOUND on)
else(NOT segwayrmp_INCLUDE_DIRS OR NOT segwayrmp_LIBRARIES)
  set(segwayrmp_FOUND off)
endif(NOT segwayrmp_INCLUDE_DIRS OR NOT segwayrmp_LIBRARIES)
