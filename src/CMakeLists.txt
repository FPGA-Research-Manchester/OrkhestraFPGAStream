# Source
file(GLOB HEADER_LIST "${DBMStoDSPI_SOURCE_DIR}/src/*.hpp")
file(GLOB SOURCE_LIST "${DBMStoDSPI_SOURCE_DIR}/src/*.cpp")

add_library(db_to_hw_library ${HEADER_LIST} ${SOURCE_LIST})

target_include_directories(db_to_hw_library INTERFACE "${DBMStoDSPI_SOURCE_DIR}/src")