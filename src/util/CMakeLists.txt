add_library(util INTERFACE)

target_sources(util INTERFACE util.hpp logger.hpp)

target_include_directories(util INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.8.5
)

FetchContent_GetProperties(spdlog)
if(NOT spdlog_POPULATED)
  FetchContent_Populate(spdlog)
  target_include_directories(util INTERFACE ${spdlog_SOURCE_DIR}/include)
endif()