#include "setup.hpp"

#include "dma.hpp"
#include "dma_setup.hpp"
#include "filter.hpp"
#include "filter_setup.hpp"

#include <unistd.h>

void Setup::SetupQueryAcceleration(int* volatile memory_pointer,

                                   std::vector<int>& db_data,
                                   int* volatile output_memory_address,
                                   int record_size, int record_count) {
  DMA dma_engine(memory_pointer);
  int input_stream_id = 0;
  int output_stream_id = 1;
  DMASetup::SetupDMAModule(dma_engine, db_data, output_memory_address,
                           record_size, record_count, input_stream_id,
                           output_stream_id);

  // Setup the filter module
  Filter filter_module(memory_pointer, 1);
  FilterSetup::SetupFilterModule(filter_module, input_stream_id,
                                 output_stream_id);

  bool input_stream_active[16] = {false};
  input_stream_active[0] = true;
  dma_engine.StartInputController(input_stream_active);
  bool output_stream_active[16] = {false};
  output_stream_active[1] = true;

  dma_engine.StartOutputController(output_stream_active);

  // Print out the contents of memory for debugging
  // std::cout << std::endl << "Memory contents:" << std::endl;
  // for (int i = 0; i < 1048576; i++) {
  //	if (memoryPointer[i] != -1) {
  //		std::cout << "Address:" << i << std::endl;
  //		std::cout << memoryPointer[i] << std::endl;
  //	}
  //}


  // check isInputControllerFinished and isOutputControllerFinished
  while (!(dma_engine.IsInputControllerFinished() &&
         dma_engine.IsOutputControllerFinished())) {
  }
}
