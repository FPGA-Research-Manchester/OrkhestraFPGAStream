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


# SQL parser
add_library(sql_parsing 
            sql_query_creator.cpp
            sql_query_creator.hpp
            sql_query_data.hpp
            sql_json_writer.cpp
            sql_json_writer.hpp
	    q19_creator.hpp
	    q19_creator.cpp
	    sql_parser.hpp
	    sql_parser.cpp
	    sql_json_reader.cpp
	    sql_json_reader.hpp)

FetchContent_Declare(
        rapidjson
        GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
        GIT_TAG        v1.1.0
)

FetchContent_GetProperties(rapidjson)
if(NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)
endif()

target_include_directories(sql_parsing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                       PUBLIC "${rapidjson_SOURCE_DIR}/include")

target_link_libraries(sql_parsing PUBLIC core_interfaces)
target_link_libraries(sql_parsing PRIVATE util)
