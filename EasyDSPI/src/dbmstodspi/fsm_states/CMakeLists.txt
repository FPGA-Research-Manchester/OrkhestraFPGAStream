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