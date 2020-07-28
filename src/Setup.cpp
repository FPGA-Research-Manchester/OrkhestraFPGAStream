#include "Setup.hpp"

#include "DMA.hpp"
#include "DMASetup.hpp"
#include "Filter.hpp"
#include "FilterSetup.hpp"
void Setup::SetupQueryAcceleration(int* volatile& memoryPointer,
                                   std::vector<int>& dbData, int recordSize,
                                   int recordCount) {
  DMA dma_engine(memoryPointer);
  int input_stream_id = 0;
  int output_stream_id = 1;
  DMASetup dma_setup;
  DMASetup::SetupDMAModule(dma_engine, dbData, recordSize, recordCount,
                           input_stream_id, output_stream_id);

  // Setup the filter module
  Filter filter_module(memoryPointer, 1);
  FilterSetup filter_setup;
  FilterSetup::SetupFilterModule(filter_module, input_stream_id,
                                 output_stream_id);

  bool stream_active[16] = {false};
  stream_active[0] = true;
  dma_engine.startInputController(stream_active);
  dma_engine.startOutputController(stream_active);

  // Print out the contents of memory for debugging
  // std::cout << std::endl << "Memory contents:" << std::endl;
  // for (int i = 0; i < 1048576; i++) {
  //	if (memoryPointer[i] != -1) {
  //		std::cout << "Address:" << i << std::endl;
  //		std::cout << memoryPointer[i] << std::endl;
  //	}
  //}

  // check isInputControllerFinished and isOutputControllerFinished
}