# Tests
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
FetchContent_MakeAvailable(googletest)

file(GLOB HEADER_LIST "${DBMStoDSPI_SOURCE_DIR}/tests/*.hpp")
file(GLOB SOURCE_LIST "${DBMStoDSPI_SOURCE_DIR}/tests/*.cpp")

add_executable(testlib ${HEADER_LIST} ${SOURCE_LIST}) 

target_link_libraries(testlib 
	                  gtest 
					  gtest_main
					  gmock
					  gmock_main
					  fpga_managing
					  data_managing # I guess the data_managing and fos would be tested in a different lib
					  fos
)

target_include_directories(testlib INTERFACE "${DBMStoDSPI_SOURCE_DIR}/tests")

file(COPY ${DBMStoDSPI_SOURCE_DIR}/tests/resources/DMACrossbarSetupTest DESTINATION .)

add_test(NAME AccelerationModuleTest COMMAND testlib)
add_test(NAME DMACrossbarSetupTest COMMAND testlib)
add_test(NAME FilterSetupTest COMMAND testlib)
add_test(NAME DMASetupTest COMMAND testlib)

add_custom_command(TARGET testlib
        POST_BUILD
		COMMAND ctest 
        )