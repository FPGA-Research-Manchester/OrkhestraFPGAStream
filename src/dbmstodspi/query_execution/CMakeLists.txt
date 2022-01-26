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

if (ENABLE_FPGA)
	file(GLOB OS_DEPENDENT_MEMORY_BLOCK "table_data/udma_memory_block.cpp" "table_data/udma_memory_block.hpp" "table_data/virtual_memory_block.hpp" "table_data/virtual_memory_block.cpp")
else()
	file(GLOB OS_DEPENDENT_MEMORY_BLOCK "table_data/virtual_memory_block.hpp" "table_data/virtual_memory_block.cpp")
endif()

# DBMStoDSPI::FSM_states
add_library(query_execution OBJECT 
            states/execute_state.hpp
            states/execute_state.cpp
            states/print_plan_state.hpp
            states/print_plan_state.cpp
            states/schedule_state.hpp
            states/schedule_state.cpp
            states/debug_schedule_state.hpp
            states/debug_schedule_state.cpp
            states/setup_nodes_state.hpp
            states/setup_nodes_state.cpp
            states/state_interface.hpp
            states/setup_scheduling_state.cpp
            states/setup_scheduling_state.hpp
            graph_processing_fsm_interface.hpp
            query_acceleration_constants.hpp
            table_data/csv_reader.cpp
            table_data/csv_reader.hpp
            table_data/csv_reader_interface.hpp
            table_data/data_manager.cpp
            table_data/data_manager.hpp
            table_data/data_manager_interface.hpp
            table_data/types_converter.cpp
            table_data/types_converter.hpp
            table_data/memory_manager.cpp
            table_data/memory_manager.hpp
            table_data/memory_manager_interface.hpp
            fpga_managing/fpga_manager.hpp
            fpga_managing/fpga_manager.cpp
            fpga_managing/fpga_manager_interface.hpp
            fpga_managing/fpga_driver_factory.hpp
            fpga_managing/fpga_driver_factory.cpp
            fpga_managing/fpga_driver_factory_interface.hpp
            fpga_managing/module_config_values.hpp
            fpga_managing/accelerated_query_node.hpp
            fpga_managing/stream_data_parameters.hpp
            fpga_managing/dma_crossbar_setup_data.hpp
            fpga_managing/dma_crossbar_specifier.hpp
            fpga_managing/dma_crossbar_specifier.cpp
            fpga_managing/dma_setup_data.hpp
            fpga_managing/stream_parameter_calculator.hpp
            fpga_managing/stream_parameter_calculator.cpp
            fpga_managing/accelerator_library.hpp
            fpga_managing/accelerator_library.cpp
            fpga_managing/accelerator_library_interface.hpp
            fpga_managing/modules/acceleration_module.cpp
            fpga_managing/modules/acceleration_module.hpp
            fpga_managing/modules/addition.cpp
            fpga_managing/modules/addition.hpp
            fpga_managing/modules/addition_interface.hpp
            fpga_managing/modules/aggregation_sum.cpp
            fpga_managing/modules/aggregation_sum.hpp
            fpga_managing/modules/aggregation_sum_interface.hpp
            fpga_managing/modules/dma.cpp
            fpga_managing/modules/dma.hpp
            fpga_managing/modules/dma_interface.hpp
            fpga_managing/modules/dma.cpp
            fpga_managing/modules/dma.hpp
            fpga_managing/modules/dma_interface.hpp
            fpga_managing/modules/filter.cpp
            fpga_managing/modules/filter.hpp
            fpga_managing/modules/filter_interface.hpp
            fpga_managing/modules/join.cpp
            fpga_managing/modules/join.hpp
            fpga_managing/modules/join_interface.hpp
            fpga_managing/modules/join.cpp
            fpga_managing/modules/join.hpp
            fpga_managing/modules/join_interface.hpp
            fpga_managing/modules/linear_sort.cpp
            fpga_managing/modules/linear_sort.hpp
            fpga_managing/modules/linear_sort_interface.hpp
            fpga_managing/modules/merge_sort.cpp
            fpga_managing/modules/merge_sort.hpp
            fpga_managing/modules/merge_sort_interface.hpp
            fpga_managing/modules/multiplication.cpp
            fpga_managing/modules/multiplication.hpp
            fpga_managing/modules/multiplication_interface.hpp
            fpga_managing/modules/ila.cpp
            fpga_managing/modules/ila.hpp
            fpga_managing/modules/ila_types.hpp
            fpga_managing/modules/decorators/read_back_module.hpp
            fpga_managing/setup/addition_setup.cpp
            fpga_managing/setup/addition_setup.hpp
            fpga_managing/setup/aggregation_sum_setup.cpp
            fpga_managing/setup/aggregation_sum_setup.hpp
            fpga_managing/setup/dma_crossbar_setup.cpp
            fpga_managing/setup/dma_crossbar_setup.hpp
            fpga_managing/setup/dma_setup.cpp
            fpga_managing/setup/dma_setup.hpp
            fpga_managing/setup/dma_setup_interface.hpp
            fpga_managing/setup/filter_setup.cpp
            fpga_managing/setup/filter_setup.hpp
            fpga_managing/setup/join_setup.cpp
            fpga_managing/setup/join_setup.hpp
            fpga_managing/setup/linear_sort_setup.cpp
            fpga_managing/setup/linear_sort_setup.hpp
            fpga_managing/setup/merge_sort_setup.cpp
            fpga_managing/setup/merge_sort_setup.hpp
            fpga_managing/setup/multiplication_setup.cpp
            fpga_managing/setup/multiplication_setup.hpp
            fpga_managing/setup/acceleration_module_setup_interface.hpp
            fpga_managing/setup/acceleration_module_setup_interface.cpp
		    fpga_managing/setup/decorators/first_module_setup.cpp
            fpga_managing/setup/decorators/first_module_setup.hpp
		    fpga_managing/setup/decorators/sorting_module_setup.cpp
		    fpga_managing/setup/decorators/sorting_module_setup.hpp
            fpga_managing/setup/decorators/blocking_sort_module_setup.hpp
            fpga_managing/setup/decorators/blocking_sort_module_setup.cpp
		    fpga_managing/setup/decorators/reducing_module_setup.hpp
		    fpga_managing/setup/decorators/reducing_module_setup.cpp
            scheduling/one_plan_node_scheduler.cpp
            scheduling/one_plan_node_scheduler.hpp
            scheduling/node_scheduler_interface.hpp
            scheduling/elastic_resource_scheduler.cpp
            scheduling/elastic_resource_scheduler.hpp
            scheduling/plan_evaluator_interface.hpp
            scheduling/plan_evaluator.cpp
            scheduling/plan_evaluator.hpp
            scheduling/simple_plan_evaluator.cpp
            scheduling/simple_plan_evaluator.hpp
            scheduling/run_linker.cpp
            scheduling/run_linker.hpp
            scheduling/elastic_scheduling_graph_parser.cpp
            scheduling/elastic_scheduling_graph_parser.hpp
            scheduling/query_manager.hpp
            scheduling/query_manager.cpp
            scheduling/query_manager_interface.hpp
            scheduling/elastic_module_checker.hpp
            scheduling/elastic_module_checker.cpp
            scheduling/id_manager.hpp
            scheduling/id_manager.cpp
            scheduling/table_manager.hpp
            scheduling/table_manager.cpp
		    scheduling/pre_scheduling_processor.cpp
		    scheduling/pre_scheduling_processor.hpp
            scheduling/scheduling_query_node.hpp
            scheduling/query_scheduling_helper.hpp
            scheduling/query_scheduling_helper.cpp
            scheduling/scheduled_module.hpp
            scheduling/module_selection.cpp
            scheduling/module_selection.hpp
            scheduling/scheduling_data.hpp
            ${OS_DEPENDENT_MEMORY_BLOCK})

target_include_directories(query_execution PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} 
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/states
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/table_data
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/fpga_managing
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/fpga_managing/modules
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/fpga_managing/setup
		                                          ${CMAKE_CURRENT_SOURCE_DIR}/fpga_managing/modules/decorators
		                                          ${CMAKE_CURRENT_SOURCE_DIR}/fpga_managing/setup/decorators
					                              ${CMAKE_CURRENT_SOURCE_DIR}/scheduling)

target_link_libraries(query_execution PRIVATE core_interfaces)
target_link_libraries(query_execution PRIVATE util)

if (ENABLE_FPGA)
	target_link_libraries(query_execution PUBLIC fos)
endif()
