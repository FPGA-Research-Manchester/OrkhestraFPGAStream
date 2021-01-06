# Source
if (NOT WIN32)
	file(GLOB OS_DEPENDENT_MEMORY_BLOCK "udma_memory_block.cpp" "udma_memory_block.hpp")
else()
	file(GLOB OS_DEPENDENT_MEMORY_BLOCK "virtual_memory_block.hpp" "virtual_memory_block.cpp")
endif()


add_library(fpga_managing 
            "query_acceleration_constants.hpp"
			"acceleration_module.cpp"  
			"acceleration_module.hpp"
			"join_interface.hpp"
			"join.cpp"
			"join.hpp"
			"join_setup.cpp"
			"join_setup.hpp"
			"dma_interface.hpp"
			"dma.hpp"
			"dma.cpp"
			"dma_setup.cpp"
			"dma_setup.hpp"
			"dma_setup_data.hpp"
			"dma_crossbar_setup.cpp"
			"dma_crossbar_setup.hpp"
			"dma_crossbar_setup_data.hpp"
			"filter_interface.hpp"
			"filter.cpp"                        
			"filter.hpp"  
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
            "merge_sort.hpp"
            "merge_sort.cpp"
			"merge_sort_interface.hpp"
			"merge_sort_setup.cpp"
			"merge_sort_setup.hpp"
			"operation_types.hpp"
			"dma_multi_channel_setup_data.hpp"
			"accelerated_query_node.hpp"
            "linear_sort.cpp"
            "linear_sort.hpp"
            "linear_sort_interface.hpp"
            "linear_sort_setup.hpp"
            "linear_sort_setup.cpp")

target_include_directories(fpga_managing PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if (NOT WIN32)
	target_link_libraries(fpga_managing PUBLIC fos)
endif()