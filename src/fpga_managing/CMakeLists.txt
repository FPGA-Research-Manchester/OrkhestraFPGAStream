# Source
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
			"udma_memory_block.cpp"
			"udma_memory_block.hpp")

target_include_directories(fpga_managing PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(fpga_managing PUBLIC fos)