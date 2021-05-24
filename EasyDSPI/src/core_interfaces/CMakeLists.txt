# Core interface
# Can be placed to an include folder to use managers standalone without the core.
set(header_files
    ${CMAKE_CURRENT_SOURCE_DIR}/config.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/execution_manager_interface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/execution_plan_graph_interface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/execution_plan_node.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/input_manager_interface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/output_manager_interface.hpp)

add_library(core_interfaces INTERFACE)

target_sources(core_interfaces INTERFACE ${header_files})

target_include_directories(core_interfaces INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})