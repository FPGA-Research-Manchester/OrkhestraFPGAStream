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
	${Prototype_SOURCE_DIR}/resources/data/CAR_DATA.csv
	${Prototype_SOURCE_DIR}/resources/data/CAR_DATA_HALF_SORTED.csv
    ${Prototype_SOURCE_DIR}/resources/data/CAR_DATA_SORTED.csv
    
    ${Prototype_SOURCE_DIR}/resources/data/CAR_DATA_8K.csv
    ${Prototype_SOURCE_DIR}/resources/data/CAR_DATA_HALF_SORTED_8K_512WAY.csv
    ${Prototype_SOURCE_DIR}/resources/data/CAR_DATA_HALF_SORTED_8K_128WAY.csv
    ${Prototype_SOURCE_DIR}/resources/data/CAR_DATA_HALF_SORTED_8K_64WAY.csv
    ${Prototype_SOURCE_DIR}/resources/data/CAR_DATA_SORTED_8K.csv
    
    ${Prototype_SOURCE_DIR}/resources/data/CAR_FILTER_DATA.csv
    ${Prototype_SOURCE_DIR}/resources/data/CUSTOMER_DATA.csv
    ${Prototype_SOURCE_DIR}/resources/data/JOIN_DATA.csv
    ${Prototype_SOURCE_DIR}/resources/data/FILTERED_JOIN_DATA.csv
    
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_empty.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_empty.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_joining.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_joining.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_filtering.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_filtering.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_filter_join.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_filter_join.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_merge_sorting.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_merge_sorting.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_big_merge_sorting.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_big_merge_sorting.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_linear_sorting.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_linear_sorting.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_filtering_linear_sort.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_filtering_linear_sort.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_double_merge_sorting.bin 
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_double_merge_sorting.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_sort_join_filter.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_sort_join_filter.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_addition.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_addition.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_multiplication.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_multiplication.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_aggregation_sum.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_aggregation_sum.json
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_add_mul_sum.bin
    ${Prototype_SOURCE_DIR}/resources/hardware/DSPI_add_mul_sum.json

#${Prototype_SOURCE_DIR}/resources/data/SF03/lineitem_sf0_3.csv
#${Prototype_SOURCE_DIR}/resources/data/SF03/lineitem_sf0_3_filter.csv
#${Prototype_SOURCE_DIR}/resources/data/SF03/lineitem_sf0_3_linear_sort.csv
#${Prototype_SOURCE_DIR}/resources/data/SF03/lineitem_sf0_3_sort.csv 
#${Prototype_SOURCE_DIR}/resources/data/SF03/part_sf0_3.csv 
#${Prototype_SOURCE_DIR}/resources/data/SF03/lineitem_part_sf0_3_1st_filter.csv 
#${Prototype_SOURCE_DIR}/resources/data/SF03/lineitem_part_sf0_3_2nd_filter.csv 
#${Prototype_SOURCE_DIR}/resources/data/SF03/lineitem_part_sf0_3_inverted.csv 
#${Prototype_SOURCE_DIR}/resources/data/SF03/lineitem_part_sf0_3_multiplied.csv 
#${Prototype_SOURCE_DIR}/resources/data/SF03/lineitem_part_sf0_3_multiplied_reduced.csv 

    ${Prototype_SOURCE_DIR}/resources/data/lineitem_sf0_1.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_sf0_1_filter.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_sf0_1_linear_sort.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_sf0_1_sort.csv 
    ${Prototype_SOURCE_DIR}/resources/data/part_sf0_1.csv 
    ${Prototype_SOURCE_DIR}/resources/data/part_sf0_1_filter.csv
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_part_sf0_1_1st_filter.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_part_sf0_1_2nd_filter.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_part_sf0_1_inverted.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_part_sf0_1_multiplied.csv 
    
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_sf0_01.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_sf0_01_filter.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_sf0_01_linear_sort.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_sf0_01_sort.csv 
    ${Prototype_SOURCE_DIR}/resources/data/part_sf0_01.csv 
    ${Prototype_SOURCE_DIR}/resources/data/part_sf0_01_filter.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_part_sf0_01_1st_filter.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_part_sf0_01_2nd_filter.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_part_sf0_01_inverted.csv 
    ${Prototype_SOURCE_DIR}/resources/data/lineitem_part_sf0_01_multiplied.csv 
    
    ${Prototype_SOURCE_DIR}/resources/data_type_sizes.json 
    ${Prototype_SOURCE_DIR}/resources/required_memory_space.json 
    ${Prototype_SOURCE_DIR}/resources/supported_accelerators.json
    ${Prototype_SOURCE_DIR}/resources/extended_accelerator_set.json 
    ${Prototype_SOURCE_DIR}/resources/big_accelerator_set.json 
    ${Prototype_SOURCE_DIR}/resources/repo.json 
    ${Prototype_SOURCE_DIR}/resources/config.ini
    ${Prototype_SOURCE_DIR}/resources/extended_config.ini 
    
    ${Prototype_SOURCE_DIR}/resources/input_defs/concurrency_testing.json 
    ${Prototype_SOURCE_DIR}/resources/input_defs/double_run_testing.json 
    ${Prototype_SOURCE_DIR}/resources/input_defs/filter_join_testing.json
    ${Prototype_SOURCE_DIR}/resources/input_defs/filter_testing.json 
    ${Prototype_SOURCE_DIR}/resources/input_defs/single_run_testing.json 
    ${Prototype_SOURCE_DIR}/resources/input_defs/TPCH_Q19_SF001.json 
    ${Prototype_SOURCE_DIR}/resources/input_defs/TPCH_Q19_SF01.json 
    ${Prototype_SOURCE_DIR}/resources/input_defs/TPCH_Q19_SF03.json 
    ${Prototype_SOURCE_DIR}/resources/input_defs/merge_sort_testing.json 
    
    ${Prototype_SOURCE_DIR}/resources/benchmark/TPCH_Q6_SF001.json 
    ${Prototype_SOURCE_DIR}/resources/benchmark/TPCH_Q6_SF1.json 
    
    ${Prototype_SOURCE_DIR}/resources/benchmark/benchmark_Q19_SF001.json 
    ${Prototype_SOURCE_DIR}/resources/benchmark/benchmark_Q19_SF1.json 
    ${Prototype_SOURCE_DIR}/resources/benchmark/lineitem0.01.csv 
    ${Prototype_SOURCE_DIR}/resources/benchmark/part0.01.csv
    
    ${Prototype_SOURCE_DIR}/resources/benchmark/benchmark.pl
    ${Prototype_SOURCE_DIR}/resources/benchmark/benchmark_defs.zip 
#${Prototype_SOURCE_DIR}/resources/benchmark/lineitem.zip 
#${Prototype_SOURCE_DIR}/resources/benchmark/part.zip 
    ${Prototype_SOURCE_DIR}/resources/benchmark/big_benchmark_config.ini 
    ${Prototype_SOURCE_DIR}/resources/benchmark/fast_benchmark_config.ini
    ${Prototype_SOURCE_DIR}/resources/benchmark/slow_benchmark_config.ini)

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