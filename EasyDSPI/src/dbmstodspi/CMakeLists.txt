# DBMStoDSPI

add_subdirectory(input_handling)
add_subdirectory(fsm_states)

add_library(dbmstodspi 
            $<TARGET_OBJECTS:input_handling>
            $<TARGET_OBJECTS:fsm_states>)

target_link_libraries(dbmstodspi PUBLIC input_handling
                                 PUBLIC fsm_states)