# DBMStoDSPI::Input_handling
add_library(input_handling OBJECT 
            graph_creator.cpp
            graph_creator.hpp
            graph.cpp
            graph.hpp
            config_value_checker_interface.hpp
            graph_creator_interface.hpp
            json_reader_interface.hpp
            json_validator_interface.hpp
            rapidjson_reader.cpp
            rapidjson_reader.hpp
            input_config_reader_interface.hpp
            input_config_reader.hpp
            input_config_reader.cpp)

FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG        v1.1.0
)

FetchContent_GetProperties(rapidjson)
if(NOT rapidjson_POPULATED)
  FetchContent_Populate(rapidjson)
endif()

target_include_directories(input_handling PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                          PUBLIC "${rapidjson_SOURCE_DIR}/include")

target_link_libraries(input_handling PRIVATE core_interfaces)