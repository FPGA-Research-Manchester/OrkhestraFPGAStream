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


# Source
add_library(query_managing 
            "query_manager.hpp"
            "query_manager.cpp"
            "query_scheduling_data.hpp"
            "table_manager.cpp"
            "table_manager.hpp"
            "node_scheduler.cpp"
            "node_scheduler.hpp"
            "id_manager.hpp"
            "id_manager.cpp"
            "elastic_module_checker.hpp"
            "elastic_module_checker.cpp")

target_include_directories(query_managing PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(query_managing PUBLIC fpga_managing)
target_link_libraries(query_managing PUBLIC data_managing)
target_link_libraries(query_managing PUBLIC input_managing)
target_link_libraries(query_managing PRIVATE util)
if (NOT WIN32)
	target_link_libraries(query_managing PUBLIC fos)
endif()