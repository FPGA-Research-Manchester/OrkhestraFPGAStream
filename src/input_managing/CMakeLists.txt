add_library(input_managing
            graph_creator.cpp
            graph_creator.hpp
            rapidjson_reader.cpp
            rapidjson_reader.hpp
            json_reader_interface.hpp
            config.hpp
            input_config_reader.cpp
            input_config_reader.hpp
            input_config_reader_interface.hpp)

FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG        v1.1.0
)

FetchContent_GetProperties(rapidjson)
if(NOT rapidjson_POPULATED)
  FetchContent_Populate(rapidjson)
endif()

target_include_directories(input_managing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                          PUBLIC "${rapidjson_SOURCE_DIR}/include")

target_link_libraries(input_managing PRIVATE query_managing)