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


# DBMStoDSPI

add_subdirectory(input_handling)
add_subdirectory(fsm_states)

add_library(dbmstodspi 
            $<TARGET_OBJECTS:input_handling>
            $<TARGET_OBJECTS:fsm_states>)

target_link_libraries(dbmstodspi PUBLIC input_handling
                                 PUBLIC fsm_states)