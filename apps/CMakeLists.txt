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
	${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/CAR_DATA.csv
	${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/CAR_DATA_HALF_SORTED.csv
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/CAR_DATA_SORTED.csv
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/CAR_DATA_8K.csv
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/CAR_DATA_HALF_SORTED_8K_512WAY.csv
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/CAR_DATA_HALF_SORTED_8K_128WAY.csv
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/CAR_DATA_HALF_SORTED_8K_64WAY.csv
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/CAR_DATA_SORTED_8K.csv
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/CAR_FILTER_DATA.csv
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/CUSTOMER_DATA.csv
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/JOIN_DATA.csv
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/FILTERED_JOIN_DATA.csv
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_empty.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_empty.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_joining.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_joining.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_filtering.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_filtering.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_filter_join.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_filter_join.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_merge_sorting.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_merge_sorting.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_big_merge_sorting.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_big_merge_sorting.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_linear_sorting.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_linear_sorting.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_filtering_linear_sort.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_filtering_linear_sort.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_double_merge_sorting.bin 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_double_merge_sorting.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_sort_join_filter.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_sort_join_filter.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_addition.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_addition.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_multiplication.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_multiplication.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_aggregation_sum.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_aggregation_sum.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_add_mul_sum.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_add_mul_sum.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_join_filter.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/DSPI_join_filter.json

#${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/SF03/lineitem_sf0_3.csv
#${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/SF03/lineitem_sf0_3_filter.csv
#${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/SF03/lineitem_sf0_3_linear_sort.csv
#${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/SF03/lineitem_sf0_3_sort.csv 
#${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/SF03/part_sf0_3.csv 
#${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/SF03/lineitem_part_sf0_3_1st_filter.csv 
#${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/SF03/lineitem_part_sf0_3_2nd_filter.csv 
#${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/SF03/lineitem_part_sf0_3_inverted.csv 
#${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/SF03/lineitem_part_sf0_3_multiplied.csv 
#${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/SF03/lineitem_part_sf0_3_multiplied_reduced.csv 

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_sf0_1.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_sf0_1_filter.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_sf0_1_linear_sort.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_sf0_1_sort.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/part_sf0_1.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/part_sf0_1_filter.csv
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_part_sf0_1_1st_filter.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_part_sf0_1_2nd_filter.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_part_sf0_1_inverted.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_part_sf0_1_multiplied.csv 
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_sf0_01.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_sf0_01_filter.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_sf0_01_linear_sort.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_sf0_01_sort.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/part_sf0_01.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/part_sf0_01_filter.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_part_sf0_01_1st_filter.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_part_sf0_01_2nd_filter.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_part_sf0_01_inverted.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data/lineitem_part_sf0_01_multiplied.csv 
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/data_type_sizes.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/required_memory_space.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/supported_accelerators.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/extended_accelerator_set.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/big_accelerator_set.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/repo.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/config.ini
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/extended_config.ini 
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/concurrency_testing.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/double_run_testing.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/filter_join_testing.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/filter_testing.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/single_run_testing.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/TPCH_Q19_SF001.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/TPCH_Q19_SF01.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/TPCH_Q19_SF03.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/merge_sort_testing.json 
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/TPCH_Q6_SF001.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/TPCH_Q6_SF1.json 
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_Q19_SF001.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_Q19_SF1.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/lineitem0.01.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/part0.01.csv
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark.pl
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_defs.zip 
#${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/lineitem.zip 
#${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/part.zip 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/big_benchmark_config.ini 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/fast_benchmark_config.ini
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/slow_benchmark_config.ini

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/tables_data.json)

add_executable (OrkhestraFPGAStream main.cpp ${data})

FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG        v2.2.1
)

FetchContent_GetProperties(cxxopts)
if(NOT cxxopts_POPULATED)
  FetchContent_Populate(cxxopts)
  target_include_directories(OrkhestraFPGAStream PRIVATE ${cxxopts_SOURCE_DIR}/include)
endif()

target_link_libraries(OrkhestraFPGAStream PRIVATE core)
target_link_libraries(OrkhestraFPGAStream PRIVATE util)

file(COPY ${data} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})