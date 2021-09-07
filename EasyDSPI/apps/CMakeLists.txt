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


# Apps
# This list is for moving example testing data.
set(data 
	${Prototype_SOURCE_DIR}/resources/CAR_DATA.csv
	${Prototype_SOURCE_DIR}/resources/CAR_FILTER_DATA.csv
	${Prototype_SOURCE_DIR}/resources/config.ini
	${Prototype_SOURCE_DIR}/resources/data_type_sizes.json

	${Prototype_SOURCE_DIR}/resources/DSPI_add_mul_sum.bin
	${Prototype_SOURCE_DIR}/resources/DSPI_add_mul_sum.json
	${Prototype_SOURCE_DIR}/resources/DSPI_addition.bin
	${Prototype_SOURCE_DIR}/resources/DSPI_addition.json
	${Prototype_SOURCE_DIR}/resources/DSPI_aggregation_sum.bin
	${Prototype_SOURCE_DIR}/resources/DSPI_aggregation_sum.json
	${Prototype_SOURCE_DIR}/resources/DSPI_empty.bin
	${Prototype_SOURCE_DIR}/resources/DSPI_empty.json
	${Prototype_SOURCE_DIR}/resources/DSPI_filter_join.bin
	${Prototype_SOURCE_DIR}/resources/DSPI_filter_join.json
	${Prototype_SOURCE_DIR}/resources/DSPI_filtering.bin
	${Prototype_SOURCE_DIR}/resources/DSPI_filtering.json
	${Prototype_SOURCE_DIR}/resources/DSPI_filtering_linear_sort.bin
	${Prototype_SOURCE_DIR}/resources/DSPI_filtering_linear_sort.json
	${Prototype_SOURCE_DIR}/resources/DSPI_joining.bin
	${Prototype_SOURCE_DIR}/resources/DSPI_joining.json
	${Prototype_SOURCE_DIR}/resources/DSPI_linear_sorting.bin
	${Prototype_SOURCE_DIR}/resources/DSPI_linear_sorting.json
	${Prototype_SOURCE_DIR}/resources/DSPI_merge_sorting.bin
	${Prototype_SOURCE_DIR}/resources/DSPI_merge_sorting.json
	${Prototype_SOURCE_DIR}/resources/DSPI_multiplication.bin
	${Prototype_SOURCE_DIR}/resources/DSPI_multiplication.json
	${Prototype_SOURCE_DIR}/resources/DSPI_sort_join_filter.bin
	${Prototype_SOURCE_DIR}/resources/DSPI_sort_join_filter.json

	${Prototype_SOURCE_DIR}/resources/filter_testing.json
	${Prototype_SOURCE_DIR}/resources/repo.json
	${Prototype_SOURCE_DIR}/resources/required_memory_space.json
	${Prototype_SOURCE_DIR}/resources/supported_accelerators.json)

add_executable (Prototype main.cpp ${data})

FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG        v2.2.1
)

FetchContent_GetProperties(cxxopts)
if(NOT cxxopts_POPULATED)
  FetchContent_Populate(cxxopts)
  target_include_directories(Prototype PRIVATE ${cxxopts_SOURCE_DIR}/include)
endif()

target_link_libraries(Prototype PRIVATE core)
target_link_libraries(Prototype PRIVATE util)

file(COPY ${data} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})