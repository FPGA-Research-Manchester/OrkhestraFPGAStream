# DBMStoDSPI
add_library(dbmstodspi 
            graph_creator.cpp
            graph_creator.hpp
            graph.cpp
            graph.hpp)
target_include_directories(dbmstodspi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dbmstodspi PRIVATE core_interfaces)