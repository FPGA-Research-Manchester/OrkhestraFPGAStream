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
  GIT_TAG        release-1.11.0
)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE HEADER_LIST "${OrkhestraFPGAStream_SOURCE_DIR}/tests/*.hpp")
file(GLOB_RECURSE SOURCE_LIST "${OrkhestraFPGAStream_SOURCE_DIR}/tests/*.cpp")

add_executable(testlib ${HEADER_LIST} ${SOURCE_LIST}) 

target_link_libraries(testlib 
	                  gtest 
					  gtest_main
					  gmock
					  gmock_main
					  dbmstodspi
					  core_interfaces
)

if (NOT WIN32)
	target_link_libraries(testlib fos)
endif()

target_include_directories(testlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core
										  ${CMAKE_CURRENT_SOURCE_DIR}/core/core_execution
										  ${CMAKE_CURRENT_SOURCE_DIR}/core/core_input
										  ${CMAKE_CURRENT_SOURCE_DIR}/dbmstodspi/query_execution/states
                                          ${CMAKE_CURRENT_SOURCE_DIR}/dbmstodspi/query_execution/table_data
                                          ${CMAKE_CURRENT_SOURCE_DIR}/dbmstodspi/query_execution/fpga_managing
                                          ${CMAKE_CURRENT_SOURCE_DIR}/dbmstodspi/query_execution/fpga_managing/modules
                                          ${CMAKE_CURRENT_SOURCE_DIR}/dbmstodspi/query_execution/fpga_managing/setup
					                      ${CMAKE_CURRENT_SOURCE_DIR}/dbmstodspi/query_execution/scheduling
										  ${CMAKE_CURRENT_SOURCE_DIR}/dbmstodspi/util
										  ${CMAKE_CURRENT_SOURCE_DIR}/dbmstodspi/input_handling)

target_include_directories(testlib INTERFACE "${OrkhestraFPGAStream_SOURCE_DIR}/tests")

file(COPY ${OrkhestraFPGAStream_SOURCE_DIR}/tests/dbmstodspi/query_execution/fpga_managing/setup/resources/DMACrossbarSetupTest DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME DMACrossbarSpecifierTest COMMAND testlib)
add_test(NAME AccelerationModuleTest COMMAND testlib)
add_test(NAME DMACrossbarSetupTest COMMAND testlib)
add_test(NAME FilterSetupTest COMMAND testlib)
add_test(NAME DMASetupTest COMMAND testlib)
add_test(NAME NodeSchedulerTest COMMAND testlib)
add_test(NAME TypesConverterTest COMMAND testlib)
add_test(NAME IDManagerTest COMMAND testlib)
add_test(NAME ElasticModuleCheckerTest COMMAND testlib)
add_test(NAME DataManagerTest COMMAND testlib)
add_test(NAME TableManagerTest COMMAND testlib)

# Uncomment for automatic testing
#add_custom_command(TARGET testlib
#        POST_BUILD
#		COMMAND ctest 
#        )