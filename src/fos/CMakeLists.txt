# Picked src files from https://github.com/FPGA-Research-Manchester/fos
link_libraries(stdc++fs)
add_library(fos 
			"cynq.cpp"  
			"cynq.h"
			"fpga.h"
			"json.hpp"  
			"mmio.cpp"  
			"mmio.h"
			"udma.cpp"  
			"udma.h")

target_include_directories(fos PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")