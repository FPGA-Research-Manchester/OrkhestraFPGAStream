# Apps
add_executable(app main.cpp)

configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/MOCK_DATA.csv MOCK_DATA.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/RESULT_DATA.csv RESULT_DATA.csv COPYONLY)

target_link_libraries(app PRIVATE fpga_managing)
target_link_libraries(app PRIVATE data_managing)
