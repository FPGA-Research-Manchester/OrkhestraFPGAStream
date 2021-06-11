# Source
#add_library(util 
#            "util.hpp")

#target_include_directories(util PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
#set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)

add_library(util INTERFACE)

target_sources(util INTERFACE util.hpp)

target_include_directories(util INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})