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

project(OrkhestraFPGAStream  
        VERSION 0.1 
        DESCRIPTION "A DBMS FPGA acceleration configurator"
        LANGUAGES CXX C
)

#To enable clang-tidy tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#Using C++ 17 features like concatenated namespaces
set(CMAKE_CXX_STANDARD 17)

#To download external libraries like rapidcsv, rapidjson and googletest
include(FetchContent)
if (ENABLE_TESTS)
    include(CTest)
endif()
if (ENABLE_FPGA)
    add_subdirectory(src/fos)
    add_definitions(-DFPGA_AVAILABLE)
endif()

# Global dependency
add_subdirectory(src/core_interfaces)
# Implementation
add_subdirectory(src/dbmstodspi)
# Core libraries gluing everything together
add_subdirectory(src/core)
# Library responsible for SQL interfacing
add_subdirectory(src/sql_parsing)
# Applications
add_subdirectory(apps)
# Tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Build docs. Set it to ON to build
set (BUILD_DOCS OFF)
if(BUILD_DOCS)
    # From https://vicrucann.github.io/tutorials/quick-cmake-doxygen/
    # first we can indicate the documentation build as an option and set it to ON by default
    option(BUILD_DOC "Build documentation" ON)

    # check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target( doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif()