# Core

# Calls input_manager_interface -> execution_manager_interface(config, execution_plan_graph_interface)
add_subdirectory(core_input)
add_subdirectory(core_execution)
add_library(core 
            core.cpp
            core.hpp)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# Why separate libraries here? Because the factories can be changed out in the future once an interface gets added to the core_interfaces library. And into these libraries can go multiple implementations of the managers which can be constructed with the given factory. And the managers can be changed to be used standalone with the interfaces. And since they could be used standalone they're not OBJECT libraries either.
target_link_libraries(core PRIVATE core_interfaces
                           PRIVATE core_input
                           PRIVATE core_execution)