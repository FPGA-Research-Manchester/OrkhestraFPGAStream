# Source
add_library(query_managing 
            "query_manager.hpp"
            "query_manager.cpp"
            "query_scheduling_data.hpp"
            "table_manager.cpp"
            "table_manager.hpp"
            "node_scheduler.cpp"
            "node_scheduler.hpp")

target_include_directories(query_managing PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(query_managing PUBLIC fpga_managing)
target_link_libraries(query_managing PUBLIC data_managing)
if (NOT WIN32)
	target_link_libraries(query_managing PUBLIC fos)
endif()