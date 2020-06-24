# Root
cmake_minimum_required(VERSION 3.17)

project(DBMStoDSPI 
        VERSION 0.1 
        DESCRIPTION "A transparent DBMS FPGA acceleration configurator"
        LANGUAGES CXX C
)

include(FetchContent)
include(CTest)

add_subdirectory(src)
add_subdirectory(apps)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
