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
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/column_sizes.json 
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/concurrency_testing.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/double_run_testing.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/filter_join_testing.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/filter_testing.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/addition_testing.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/single_run_testing.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/TPCH_Q19_SF001.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/TPCH_Q19_SF01.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/TPCH_Q19_SF03.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/merge_sort_testing.json 

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/input_defs/lin_sort_testing.json 
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/TPCH_Q6_SF001.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/TPCH_Q6_SF1.json 
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_Q19_SF001.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_Q19_SF01.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_Q6_SF01.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_Q19_SF1.json 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_Q17_SF011.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_Q17_SF012.json
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_Q17_SF01_beg.json
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_Q17_SF01_beg1.json
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_Q17_SF01_beg2.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/lineitem0.01.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/part0.01.csv
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/lineitem0.1.csv 
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/part0.1.csv
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/lineitem0.03.csv 
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/part0.03.csv
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/lineitem0.3.csv 
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/part0.3.csv
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/part1.csv
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/lineitem1.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/lineitem_linear_sort.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/addition_input.csv 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/filter_input.csv 
    
    
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark.pl
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/benchmark_defs.zip 
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/lineitem.zip 
    #${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/part.zip 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/big_benchmark_config.ini 
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/fast_benchmark_config.ini
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/benchmark/slow_benchmark_config.ini

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/tables_data.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/pr_hw_library.json

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_Filter_7_36.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_Filter_37_66.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_Filter_67_96.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_Filter_7_36.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_Filter_37_66.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_Filter_67_96.bin

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_LinearSort512_7_36.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_LinearSort512_37_66.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_LinearSort512_67_96.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_LinearSort512_7_36.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_LinearSort512_37_66.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_LinearSort512_67_96.bin

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeJoin2K_7_36.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeJoin2K_37_66.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeJoin2K_67_96.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeJoin2K_7_36.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeJoin2K_37_66.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeJoin2K_67_96.bin

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort64_7_36.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort64_37_66.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort64_67_96.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort64_7_36.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort64_37_66.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort64_67_96.bin

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_AggregateGlobalSum_55_63.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_AggregateGlobalSum_55_63.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_ConstArith64b_85_96.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_ConstArith64b_85_96.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_DecMult64b_64_84.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_DecMult64b_64_84.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_DecMult64b_34_54.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_DecMult64b_34_54.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_DecMult64b_4_24.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_DecMult64b_4_24.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_ConstArith64b_25_36.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_ConstArith64b_25_36.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_ConstArith64b_55_66.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_ConstArith64b_55_66.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_AggregateGlobalSum_85_93.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_AggregateGlobalSum_85_93.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_AggregateGlobalSum_25_33.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_AggregateGlobalSum_25_33.bin

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/static.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/static.json

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_2.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_5.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_8.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_11.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_14.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_17.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_20.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_23.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_26.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_29.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_32.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_35.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_38.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_41.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_44.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_47.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_50.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_53.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_56.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_59.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_62.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_65.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_68.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_71.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_74.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_77.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_80.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_83.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_86.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_89.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_92.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_95.json

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_2.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_5.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_8.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_11.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_14.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_17.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_20.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_23.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_26.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_29.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_32.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_35.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_38.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_41.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_44.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_47.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_50.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_53.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_56.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_59.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_62.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_65.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_68.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_71.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_74.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_77.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_80.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_83.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_86.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_89.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_92.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/RT_95.bin

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_2.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_5.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_8.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_11.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_14.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_17.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_20.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_23.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_26.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_29.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_32.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_35.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_38.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_41.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_44.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_47.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_50.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_53.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_56.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_59.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_62.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_65.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_68.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_71.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_74.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_77.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_80.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_83.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_86.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_89.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_92.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_95.bin

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_2.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_5.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_8.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_11.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_14.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_17.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_20.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_23.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_26.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_29.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_32.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_35.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_38.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_41.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_44.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_47.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_50.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_53.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_56.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_59.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_62.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_65.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_68.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_71.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_74.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_77.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_80.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_83.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_86.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_89.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_92.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Route_through/TAA_95.json

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort32_46_66.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort32_46_66.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort32_16_36.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort32_16_36.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_LinearSort1024_37_78.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_LinearSort1024_37_78.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_LinearSort1024_7_48.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_LinearSort1024_7_48.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort128_49_96.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort128_49_96.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort128_19_66.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort128_19_66.bin
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort32_76_96.json
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/hardware/Parital_modules_latest/binPartial_MergeSort32_76_96.bin

    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/sql_input/q19.txt
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/sql_input/q6.txt
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/sql_input/q1.txt
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/sql_input/lineitem.csv
    ${OrkhestraFPGAStream_SOURCE_DIR}/resources/sql_input/part.csv

    ${OrkhestraFPGAStream_SOURCE_DIR}/extra_scripts/setupUdma.pl
    ${OrkhestraFPGAStream_SOURCE_DIR}/extra_scripts/sql_parsing/postgresql_explain_parser.py)

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
target_link_libraries(OrkhestraFPGAStream PRIVATE sql_parsing)
#set_target_properties(OrkhestraFPGAStream PROPERTIES LINK_FLAGS "/PROFILE")

file(COPY ${data} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})