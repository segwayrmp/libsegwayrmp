# If asked to and there are some test src files
if(SEGWAYRMP_BUILD_TESTS AND DEFINED SEGWAYRMP_TEST_SRCS)
  # If GTest is avialable
  find_package(GTest)
  if(GTEST_FOUND)
    message("-- Building segwayrmp Tests")
    # Add GTest to the include path
    include_directories(${GTEST_INCLUDES})
    # Compile the segwayrmp tests
    add_executable(segwayrmp_tests ${SEGWAYRMP_TEST_SRCS})
    # Link the tests to the segwayrmp library
    target_link_libraries(segwayrmp_tests ${SEGWAYRMP_TEST_LINK_LIBS}
                                               ${GTEST_LIBRARIES})
  else(GTEST_FOUND)
    message("-- Skipping segwayrmp Tests - GTest not Found!")
  endif(GTEST_FOUND)
endif(SEGWAYRMP_BUILD_TESTS AND DEFINED SEGWAYRMP_TEST_SRCS)
