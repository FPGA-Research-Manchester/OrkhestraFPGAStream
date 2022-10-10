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


add_library(util INTERFACE)

target_sources(util INTERFACE util.hpp logger.hpp orkhestra_exception.hpp)

target_include_directories(util INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.8.5
)

FetchContent_GetProperties(spdlog)
if(NOT spdlog_POPULATED)
  FetchContent_Populate(spdlog)
  target_include_directories(util INTERFACE ${spdlog_SOURCE_DIR}/include)
endif()