# Source
add_library(data_managing 
			"csv_reader.hpp"
			"csv_reader.cpp"
			"data_manager.cpp"
			"data_manager.hpp"  
			"types_converter.hpp"
			"types_converter.cpp" 
			"config_reader.cpp"
			"config_reader.hpp")

FetchContent_Declare(
  rapidcsv
  GIT_REPOSITORY https://github.com/d99kris/rapidcsv.git
  GIT_TAG        v7.00
)

FetchContent_GetProperties(rapidcsv)
if(NOT rapidcsv_POPULATED)
  FetchContent_Populate(rapidcsv)
  target_include_directories(data_managing PRIVATE ${rapidcsv_SOURCE_DIR}/src)
endif()

target_include_directories(data_managing PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")