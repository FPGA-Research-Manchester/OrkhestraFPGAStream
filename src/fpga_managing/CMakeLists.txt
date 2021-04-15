# Source
if (NOT WIN32)
	file(GLOB OS_DEPENDENT_MEMORY_BLOCK "udma_memory_block.cpp" "udma_memory_block.hpp")
else()
	file(GLOB OS_DEPENDENT_MEMORY_BLOCK "virtual_memory_block.hpp" "virtual_memory_block.cpp")
endif()


add_library(fpga_managing 
            "query_acceleration_constants.hpp"
			"modules/acceleration_module.cpp"  
			"modules/acceleration_module.hpp"
			"modules/join_interface.hpp"
			"modules/join.cpp"
			"modules/join.hpp"
			"join_setup.cpp"
			"join_setup.hpp"
			"modules/dma_interface.hpp"
			"modules/dma.hpp"
			"modules/dma.cpp"
			"dma_setup.cpp"
			"dma_setup.hpp"
			"dma_setup_data.hpp"
			"dma_crossbar_setup.cpp"
			"dma_crossbar_setup.hpp"
			"dma_crossbar_setup_data.hpp"
			"modules/filter_interface.hpp"
			"modules/filter.cpp"                        
			"modules/filter.hpp"  
			"filter_setup.cpp"
			"filter_setup.hpp"
			"filter_config_values.hpp"
			"fpga_manager.cpp"
			"fpga_manager.hpp"
			"stream_parameter_calculator.cpp"
			"stream_parameter_calculator.hpp"
			"memory_block_interface.hpp"
			"memory_manager.cpp"
			"memory_manager.hpp"
			"memory_manager_interface.hpp"
			${OS_DEPENDENT_MEMORY_BLOCK}
			"stream_data_parameters.hpp" 
			"ila.cpp"
			"ila.hpp"
			"ila_types.hpp"
			"stream_initialisation_data.hpp" 
            "modules/merge_sort.hpp"
            "modules/merge_sort.cpp"
			"modules/merge_sort_interface.hpp"
			"merge_sort_setup.cpp"
			"merge_sort_setup.hpp"
			"operation_types.hpp"
			"accelerated_query_node.hpp"
            "modules/linear_sort.cpp"
            "modules/linear_sort.hpp"
            "modules/linear_sort_interface.hpp"
            "linear_sort_setup.hpp"
            "linear_sort_setup.cpp"
			"dma_crossbar_specifier.cpp"
			"dma_crossbar_specifier.hpp"
			"modules/addition.hpp"
			"modules/addition.cpp"
			"modules/addition_interface.hpp"
			"addition_setup.hpp"
			"addition_setup.cpp"
			"modules/multiplication.hpp"
			"modules/multiplication.cpp"
			"modules/multiplication_interface.hpp"
			"multiplication_setup.hpp"
			"multiplication_setup.cpp"
			"modules/aggregation_sum.hpp"
			"modules/aggregation_sum.cpp"
			"modules/aggregation_sum_interface.hpp"
			"aggregation_sum_setup.hpp"
			"aggregation_sum_setup.cpp"
			"modules/read_back_module_interface.hpp")

target_include_directories(fpga_managing PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/modules")

if (NOT WIN32)
	target_link_libraries(fpga_managing PUBLIC fos)
endif()