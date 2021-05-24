# In theory up to 3.14 should be supported but it's tested with 3.17
cmake_minimum_required(VERSION 3.17)

project (Prototype)

#Using C++ 17 features like concatenated namespaces
set(CMAKE_CXX_STANDARD 17)

#To download external libraries like rapidcsv, rapidjson and googletest
include(FetchContent)

# Global dependency
add_subdirectory(src/core_interfaces)
# Implementation
add_subdirectory(src/dbmstodspi)
# Core libraries gluing everything together
add_subdirectory(src/core)

# This list is for moving example testing data.
set(data 
	${CMAKE_CURRENT_SOURCE_DIR}/resources/input_def.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/expected_reversed_table.csv
	${CMAKE_CURRENT_SOURCE_DIR}/resources/first_input_table.csv
	${CMAKE_CURRENT_SOURCE_DIR}/resources/config.ini
	${CMAKE_CURRENT_SOURCE_DIR}/resources/data_type_sizes.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/repo.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/supported_accelerators.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/module_drivers.json
	${CMAKE_CURRENT_SOURCE_DIR}/resources/required_memory_space.json)

add_executable (Prototype main.cpp ${data})
target_link_libraries(Prototype PRIVATE core)

file(COPY ${data} DESTINATION resources)