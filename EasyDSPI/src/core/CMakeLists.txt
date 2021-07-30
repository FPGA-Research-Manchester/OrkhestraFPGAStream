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