# Source
file(GLOB HEADER_LIST "${DBMStoDSPI_SOURCE_DIR}/src/data_managing/*.hpp")
file(GLOB SOURCE_LIST "${DBMStoDSPI_SOURCE_DIR}/src/data_managing/*.cpp")

add_library(data_managing ${HEADER_LIST} ${SOURCE_LIST})

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

target_include_directories(data_managing INTERFACE "${DBMStoDSPI_SOURCE_DIR}/src/data_managing")