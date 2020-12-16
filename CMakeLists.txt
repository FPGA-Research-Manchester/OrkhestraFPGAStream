# Root
cmake_minimum_required(VERSION 3.17)

project(DBMStoDSPI 
        VERSION 0.1 
        DESCRIPTION "A transparent DBMS FPGA acceleration configurator"
        LANGUAGES CXX C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
if (ENABLE_TESTS)
    include(CTest)
endif()

if (NOT WIN32)
    add_subdirectory(src/fos)
    # Assuming that dev work isn't done on a unix machine
    add_definitions (-D_FPGA_AVAILABLE)
endif()
add_subdirectory(src/fpga_managing)
add_subdirectory(src/data_managing)
add_subdirectory(src/query_managing)

add_subdirectory(apps)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
