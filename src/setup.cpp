#include "setup.hpp"

#include <iostream>

#include "dma.hpp"
#include "dma_setup.hpp"
#include "filter.hpp"
#include "filter_setup.hpp"

//#include <unistd.h>

void Setup::SetupQueryAcceleration(
    volatile uint32_t* configuration_memory_address,
    volatile uint32_t* input_memory_address,
    volatile uint32_t* output_memory_address, int record_size,
    int record_count) {
  DMA dma_engine(configuration_memory_address);
  int input_stream_id = 0;
  int output_stream_id = 1;
  DMASetup::SetupDMAModule(dma_engine, input_memory_address,
                           output_memory_address, record_size, record_count,
                           input_stream_id, output_stream_id);

  // Setup the filter module
  Filter filter_module(configuration_memory_address, 1);
  FilterSetup::SetupFilterModule(filter_module, input_stream_id,
                                 output_stream_id);

  bool input_stream_active[16] = {false};
  input_stream_active[0] = true;
  dma_engine.StartInputController(input_stream_active);
  bool output_stream_active[16] = {false};
  output_stream_active[1] = true;
  dma_engine.StartOutputController(output_stream_active);

  // check isInputControllerFinished and isOutputControllerFinished
  // while (!(dma_engine.IsInputControllerFinished() &&
  // dma_engine.IsOutputControllerFinished())) {
  // sleep(1);
  // std::cout<<"input:"<<dma_engine.IsInputControllerFinished()<<std::endl;
  // std::cout<<"output:"<<dma_engine.IsOutputControllerFinished()<<std::endl;
  //}
}
