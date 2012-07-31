# If asked to and there are some example src files
if(SEGWAYRMP_BUILD_EXAMPLES AND DEFINED SEGWAYRMP_EXAMPLE_SRCS)
  message("-- Building SegwayRMP Examples")
  # Compile the segwayrmp examples
  add_executable(segwayrmp_example ${SEGWAYRMP_EXAMPLE_SRCS})
  # Link the examples to link libs
  target_link_libraries(segwayrmp_example ${SEGWAYRMP_EXAMPLE_LINK_LIBS})
endif(SEGWAYRMP_BUILD_EXAMPLES AND DEFINED SEGWAYRMP_EXAMPLE_SRCS)
