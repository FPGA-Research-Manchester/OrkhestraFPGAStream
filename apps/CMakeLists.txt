# Apps
add_executable(app fpga_run.cpp)

configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/MOCK_DATA.csv MOCK_DATA.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/RESULT_DATA.csv RESULT_DATA.csv COPYONLY)

configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_filtering.bin DSPI_filtering.bin COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_filtering.json DSPI_filtering.json COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/repo.json repo.json COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/data_config.ini data_config.ini COPYONLY)

target_link_libraries(app PRIVATE fpga_managing)
target_link_libraries(app PRIVATE data_managing)
if (NOT WIN32)
	target_link_libraries(app PRIVATE fos)
endif()