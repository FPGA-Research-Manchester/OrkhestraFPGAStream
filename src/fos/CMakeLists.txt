# Picked src files from https://github.com/FPGA-Research-Manchester/fos
file(GLOB FOS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*")

link_libraries(stdc++fs)
add_library(fos ${FOS_SRC})

target_include_directories(fos PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")