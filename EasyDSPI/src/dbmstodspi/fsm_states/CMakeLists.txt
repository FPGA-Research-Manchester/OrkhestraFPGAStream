# DBMStoDSPI::FSM_states
add_library(fsm_states OBJECT 
            execute_state.hpp
            execute_state.cpp
            read_state.hpp
            read_state.cpp
            init_state.hpp
            init_state.cpp
            state_interface.hpp
            graph_processing_fsm_interface.hpp)

target_include_directories(fsm_states PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(fsm_states PRIVATE core_interfaces)