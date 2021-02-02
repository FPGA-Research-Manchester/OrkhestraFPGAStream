# Apps
add_executable(app fpga_run.cpp)

configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/CAR_DATA.csv CAR_DATA.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/CAR_DATA_HALF_SORTED.csv CAR_DATA_HALF_SORTED.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/CAR_DATA_SORTED.csv CAR_DATA_SORTED.csv COPYONLY)

configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/CAR_DATA_8K.csv CAR_DATA_8K.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/CAR_DATA_HALF_SORTED_8K_512WAY.csv CAR_DATA_HALF_SORTED_8K_512WAY.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/CAR_DATA_HALF_SORTED_8K_128WAY.csv CAR_DATA_HALF_SORTED_8K_128WAY.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/CAR_DATA_HALF_SORTED_8K_64WAY.csv CAR_DATA_HALF_SORTED_8K_64WAY.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/CAR_DATA_SORTED_8K.csv CAR_DATA_SORTED_8K.csv COPYONLY)

configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/CAR_FILTER_DATA.csv CAR_FILTER_DATA.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/CUSTOMER_DATA.csv CUSTOMER_DATA.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/CUSTOMER_DATA_FOR_JOIN.csv CUSTOMER_DATA_FOR_JOIN.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/JOIN_DATA.csv JOIN_DATA.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/FILTERED_JOIN_DATA.csv FILTERED_JOIN_DATA.csv COPYONLY)

configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_empty.bin DSPI_empty.bin COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_empty.json DSPI_empty.json COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_joining.bin DSPI_joining.bin COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_joining.json DSPI_joining.json COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_filtering.bin DSPI_filtering.bin COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_filtering.json DSPI_filtering.json COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_filter_join.bin DSPI_filter_join.bin COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_filter_join.json DSPI_filter_join.json COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_merge_sorting.bin DSPI_merge_sorting.bin COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_merge_sorting.json DSPI_merge_sorting.json COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_linear_sorting.bin DSPI_linear_sorting.bin COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_linear_sorting.json DSPI_linear_sorting.json COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_double_merge_sorting.bin DSPI_double_merge_sorting.bin COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/DSPI_double_merge_sorting.json DSPI_double_merge_sorting.json COPYONLY)

configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/tpch_orders_small.csv tpch_orders_small.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/lineitem_sf0_01.csv lineitem_sf0_01.csv COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/lineitem_sf0_1.csv lineitem_sf0_1.csv COPYONLY)

configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/lineitem_sf0_01_1st_filter.csv lineitem_sf0_01_1st_filter.csv COPYONLY)

configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/repo.json repo.json COPYONLY)
configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/data_config.ini data_config.ini COPYONLY)

target_link_libraries(app PRIVATE query_managing)