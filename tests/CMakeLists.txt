# Tests
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
FetchContent_MakeAvailable(googletest)

add_executable(testlib testing_test.cpp) 

target_link_libraries(testlib 
	                  gtest 
					  gtest_main
					  gmock
					  gmock_main
)

add_test(NAME testSet COMMAND testlib)