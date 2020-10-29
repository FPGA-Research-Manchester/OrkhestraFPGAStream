# Source
file(GLOB HEADER_LIST "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB SOURCE_LIST "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(fpga_managing ${HEADER_LIST} ${SOURCE_LIST})

target_include_directories(fpga_managing PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(fpga_managing PUBLIC fos)