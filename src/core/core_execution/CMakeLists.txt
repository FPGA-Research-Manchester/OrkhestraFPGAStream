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