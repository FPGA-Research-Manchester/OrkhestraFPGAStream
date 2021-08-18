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
add_library(data_managing 
			"csv_reader.hpp"
			"csv_reader.cpp"
			"data_manager.cpp"
			"data_manager.hpp"  
			"types_converter.hpp"
			"types_converter.cpp" 
			"table_data.hpp")

FetchContent_Declare(
  rapidcsv
  GIT_REPOSITORY https://github.com/d99kris/rapidcsv.git
  GIT_TAG        v7.00
)

FetchContent_GetProperties(rapidcsv)
if(NOT rapidcsv_POPULATED)
  FetchContent_Populate(rapidcsv)
  target_include_directories(data_managing PRIVATE ${rapidcsv_SOURCE_DIR}/src)
endif()

target_include_directories(data_managing PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(data_managing PUBLIC fpga_managing)
target_link_libraries(data_managing PUBLIC util)