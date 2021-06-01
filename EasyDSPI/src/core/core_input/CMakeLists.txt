# Core Input
add_library(core_input 
            input_manager_factory.cpp
            input_manager_factory.hpp
            input_manager.hpp
            input_manager.cpp
            input_manager_config_enums.hpp)
target_include_directories(core_input PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core_input 
                      PRIVATE core_interfaces
                      PRIVATE dbmstodspi)