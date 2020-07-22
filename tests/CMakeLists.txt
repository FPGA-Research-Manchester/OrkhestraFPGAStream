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
					  db_to_hw_library
)

target_include_directories(testlib INTERFACE "${DBMStoDSPI_SOURCE_DIR}/tests")

file(COPY ${DBMStoDSPI_SOURCE_DIR}/tests/resources/DMACrossbarSetupTest DESTINATION .)

add_test(NAME AccelerationModuleTest COMMAND testlib)
add_test(NAME DMACrossbarSetupTest COMMAND testlib)