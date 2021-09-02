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


# In theory up to 3.14 should be supported but it's tested with 3.17
cmake_minimum_required(VERSION 3.17)

project (Prototype)

#Using C++ 17 features like concatenated namespaces
set(CMAKE_CXX_STANDARD 17)

if (NOT WIN32)
    add_subdirectory(src/fos)
    # Assuming that dev work isn't done on a unix machine
    add_definitions (-D_FPGA_AVAILABLE)
endif()

#To download external libraries like rapidcsv, rapidjson and googletest
include(FetchContent)

# Global dependency
add_subdirectory(src/core_interfaces)
# Implementation
add_subdirectory(src/dbmstodspi)
# Core libraries gluing everything together
add_subdirectory(src/core)

# This list is for moving example testing data.
set(data 
	${CMAKE_CURRENT_SOURCE_DIR}/resources/CAR_DATA.csv
	${CMAKE_CURRENT_SOURCE_DIR}/resources/CAR_FILTER_DATA.csv
	${CMAKE_CURRENT_SOURCE_DIR}/resources/config.ini
	${CMAKE_CURRENT_SOURCE_DIR}/resources/data_type_sizes.json

	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_add_mul_sum.bin
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_add_mul_sum.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_addition.bin
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_addition.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_aggregation_sum.bin
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_aggregation_sum.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_empty.bin
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_empty.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_filter_join.bin
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_filter_join.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_filtering.bin
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_filtering.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_filtering_linear_sort.bin
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_filtering_linear_sort.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_joining.bin
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_joining.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_linear_sorting.bin
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_linear_sorting.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_merge_sorting.bin
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_merge_sorting.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_multiplication.bin
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_multiplication.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_sort_join_filter.bin
	${CMAKE_CURRENT_SOURCE_DIR}/resources/DSPI_sort_join_filter.json

	${CMAKE_CURRENT_SOURCE_DIR}/resources/filter_testing.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/repo.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/required_memory_space.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/supported_accelerators.json)

add_executable (Prototype main.cpp ${data})
target_link_libraries(Prototype PRIVATE core)

file(COPY ${data} DESTINATION resources)