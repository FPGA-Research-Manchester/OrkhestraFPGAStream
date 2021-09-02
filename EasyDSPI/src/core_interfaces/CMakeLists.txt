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


# Core interface
# Can be placed to an include folder to use managers standalone without the core.
set(header_files
    ${CMAKE_CURRENT_SOURCE_DIR}/config.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/execution_manager_interface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/execution_plan_graph_interface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/input_manager_interface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/operation_types.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/table_data.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/query_scheduling_data.hpp)

add_library(core_interfaces INTERFACE)

target_sources(core_interfaces INTERFACE ${header_files})

target_include_directories(core_interfaces INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})