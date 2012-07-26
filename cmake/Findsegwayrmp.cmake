find_path(segwayrmp_INCLUDE_DIRS segwayrmp/segwayrmp.h /usr/include 
          /usr/local/include "$ENV{NAMER_ROOT}")

find_library(segwayrmp_LIBRARIES segwayrmp /usr/lib /usr/local/lib
             "$ENV{NAMER_ROOT}")

if(NOT segwayrmp_INCLUDE_DIRS OR NOT segwayrmp_LIBRARIES)
  set(segwayrmp_FOUND on)
else(NOT segwayrmp_INCLUDE_DIRS OR NOT segwayrmp_LIBRARIES)
  set(segwayrmp_FOUND off)
endif(NOT segwayrmp_INCLUDE_DIRS OR NOT segwayrmp_LIBRARIES)
