# Core Execution
add_library(core_execution 
            execution_manager_factory.cpp
            execution_manager_factory.hpp
            execution_manager.hpp
            execution_manager.cpp)
target_include_directories(core_execution PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(core_execution 
                      PRIVATE core_interfaces
                      PRIVATE dbmstodspi)