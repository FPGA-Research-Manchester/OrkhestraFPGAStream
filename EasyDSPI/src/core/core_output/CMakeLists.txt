# Core Output
add_library(core_output 
            output_manager_factory.cpp
            output_manager_factory.hpp
            output_manager.hpp
            output_manager.cpp)
target_include_directories(core_output PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core_output 
                      PRIVATE core_interfaces
                      PRIVATE dbmstodspi)