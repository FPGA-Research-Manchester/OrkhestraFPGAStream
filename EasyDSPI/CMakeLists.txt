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


# In theory up to 3.14 should be supported but it's tested with 3.17
cmake_minimum_required(VERSION 3.17)

project (Prototype)

#Using C++ 17 features like concatenated namespaces
set(CMAKE_CXX_STANDARD 17)

if (NOT WIN32)
    add_subdirectory(src/fos)
    # Assuming that dev work isn't done on a unix machine
    add_definitions (-D_FPGA_AVAILABLE)
endif()

#To download external libraries like rapidcsv, rapidjson and googletest
include(FetchContent)

# Global dependency
add_subdirectory(src/core_interfaces)
# Implementation
add_subdirectory(src/dbmstodspi)
# Core libraries gluing everything together
add_subdirectory(src/core)
# Applications
add_subdirectory(apps)