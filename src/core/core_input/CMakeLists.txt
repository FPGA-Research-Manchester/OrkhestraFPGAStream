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


# Core Input
add_library(core_input 
            input_manager_factory.cpp
            input_manager_factory.hpp
            input_manager.hpp
            input_manager.cpp
	        config_creator.cpp
            config_creator.hpp)
target_include_directories(core_input PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core_input 
                      PRIVATE core_interfaces
                      PRIVATE dbmstodspi)