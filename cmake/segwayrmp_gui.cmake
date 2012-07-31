find_package(Qt4 QUIET)

if(NOT QT4_FOUND)
  message("-- ")
  message("-- SegwayRMP GUI will not be built: Qt4 not found")
  message("-- ")
endif(NOT QT4_FOUND)

find_package(SDL QUIET)

if(NOT SDL_FOUND)
  message("-- ")
  message("-- SegwayRMP GUI will not be built: SDL not found")
  message("-- ")
endif(NOT SDL_FOUND)

if(QT4_FOUND AND SDL_FOUND)
  message("-- Building SegwayRMP GUI")
  include(${QT_USE_FILE})
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  add_definitions(${QT_DEFINITIONS})

  include_directories(${SDL_INCLUDE_DIR})

  set(segwayrmp_gui_SRCS
    src/gui/main.cc
    src/gui/segwayrmp_gui.cc
  )
  set(segwayrmp_gui_HDRS
    include/segwayrmp/gui/segwayrmp_gui.h
  )
  set(segwayrmp_gui_FORMS include/segwayrmp/gui/segwayrmp_gui.ui)

  qt4_wrap_cpp(segwayrmp_gui_HDRS_MOC ${segwayrmp_gui_HDRS})
  qt4_wrap_ui(segwayrmp_gui_FORMS_HDRS ${segwayrmp_gui_FORMS})

  add_executable(segwayrmp_gui
    ${segwayrmp_gui_SRCS}
    ${segwayrmp_gui_HDRS_MOC}
    ${segwayrmp_gui_FORMS_HDRS}
  )
  target_link_libraries(segwayrmp_gui ${QT_LIBRARIES} ${SDL_LIBRARY} segwayrmp)
endif(QT4_FOUND AND SDL_FOUND)
