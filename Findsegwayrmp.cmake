find_path(segwayrmp_INCLUDE_DIRS segwayrmp.h /usr/include "$ENV{NAMER_ROOT}")

find_library(segwayrmp_LIBRARIES segwayrmp /usr/lib "$ENV{NAMER_ROOT}")

set(segwayrmp_FOUND TRUE)

if (NOT segwayrmp_INCLUDE_DIRS)
    set(segwayrmp_FOUND FALSE)
endif (NOT segwayrmp_INCLUDE_DIRS)

if (NOT segwayrmp_LIBRARIES)
    set(segwayrmp_FOUND FALSE)
endif (NOT segwayrmp_LIBRARIES)