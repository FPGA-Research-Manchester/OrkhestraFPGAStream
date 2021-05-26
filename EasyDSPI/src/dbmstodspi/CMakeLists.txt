# DBMStoDSPI
add_library(dbmstodspi 
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
            input_config_reader.cpp
            execute_state.hpp
            execute_state.cpp
            read_state.hpp
            read_state.cpp
            init_state.hpp
            init_state.cpp
            state_interface.hpp
            graph_processing_fsm_interface.hpp)

FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG        v1.1.0
)

FetchContent_GetProperties(rapidjson)
if(NOT rapidjson_POPULATED)
  FetchContent_Populate(rapidjson)
endif()

target_include_directories(dbmstodspi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                      PUBLIC "${rapidjson_SOURCE_DIR}/include")

target_link_libraries(dbmstodspi PRIVATE core_interfaces)