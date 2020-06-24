# Apps
add_executable(app main.cpp)

FetchContent_Declare(
  rapidcsv
  GIT_REPOSITORY https://github.com/d99kris/rapidcsv.git
  GIT_TAG        v7.00
)

FetchContent_GetProperties(rapidcsv)
if(NOT rapidcsv_POPULATED)
  FetchContent_Populate(rapidcsv)
  target_include_directories(app PRIVATE ${rapidcsv_SOURCE_DIR}/src)
endif()

configure_file(${DBMStoDSPI_SOURCE_DIR}/resources/MOCK_DATA.csv MOCK_DATA.csv COPYONLY)

target_link_libraries(app PRIVATE db_to_hw_library)
