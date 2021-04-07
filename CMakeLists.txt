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

# Build docs
set (BUILD_DOCS ON)
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
