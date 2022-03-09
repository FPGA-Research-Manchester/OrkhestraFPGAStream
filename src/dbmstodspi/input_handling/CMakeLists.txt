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


# DBMStoDSPI::Input_handling
add_library(input_handling OBJECT 
            graph_creator.cpp
            graph_creator.hpp
            graph.cpp
            graph.hpp
            config_value_checker_interface.hpp
            graph_creator_interface.hpp
            rapidjson_reader.cpp
            rapidjson_reader.hpp
            input_config_reader_interface.hpp
            input_config_reader.hpp
            input_config_reader.cpp)

FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG        v1.1.0
)

FetchContent_GetProperties(rapidjson)
if(NOT rapidjson_POPULATED)
  FetchContent_Populate(rapidjson)
endif()

target_include_directories(input_handling PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                          PUBLIC "${rapidjson_SOURCE_DIR}/include")

target_link_libraries(input_handling PRIVATE query_execution)
target_link_libraries(input_handling PRIVATE core_interfaces)