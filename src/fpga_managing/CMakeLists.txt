# Source
file(GLOB HEADER_LIST "${DBMStoDSPI_SOURCE_DIR}/src/fpga_managing/*.hpp")
file(GLOB SOURCE_LIST "${DBMStoDSPI_SOURCE_DIR}/src/fpga_managing/*.cpp")

file(GLOB FOS_LIST "${DBMStoDSPI_SOURCE_DIR}/src/fpga_managing/fos/*")

link_libraries(stdc++fs)
add_library(fpga_managing ${HEADER_LIST} ${SOURCE_LIST} ${FOS_LIST})

target_include_directories(fpga_managing INTERFACE "${DBMStoDSPI_SOURCE_DIR}/src/fpga_managing")