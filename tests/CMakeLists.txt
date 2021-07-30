#Copyright 2021 University of Manchester
#
#Licensed under the Apache License, Version 2.0(the "License");
#you may not use this file except in compliance with the License.
#You may obtain a copy of the License at
#
#http:  // www.apache.org/licenses/LICENSE-2.0
#
#Unless required by applicable law or agreed to in writing, software
#distributed under the License is distributed on an "AS IS" BASIS,
#WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#See the License for the specific language governing permissions and
#limitations under the License.


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
					  data_managing 
					  query_managing # I guess the data_managing, query_managing and fos (?) should be tested in a different lib
)

if (NOT WIN32)
	target_link_libraries(testlib fos)
endif()

target_include_directories(testlib INTERFACE "${DBMStoDSPI_SOURCE_DIR}/tests")

file(COPY ${DBMStoDSPI_SOURCE_DIR}/tests/resources/DMACrossbarSetupTest DESTINATION .)

add_test(NAME DMACrossbarSpecifierTest COMMAND testlib)
add_test(NAME AccelerationModuleTest COMMAND testlib)
add_test(NAME DMACrossbarSetupTest COMMAND testlib)
add_test(NAME FilterSetupTest COMMAND testlib)
add_test(NAME DMASetupTest COMMAND testlib)
add_test(NAME NodeSchedulerTest COMMAND testlib)

add_custom_command(TARGET testlib
        POST_BUILD
		COMMAND ctest 
        )