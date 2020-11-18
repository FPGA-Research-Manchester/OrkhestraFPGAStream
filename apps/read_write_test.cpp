#include "dma.hpp"

auto main() -> int {
  DMA dma_engine(reinterpret_cast<int*>(0xA0000000));
  int expected_address = 10;
  int expected_stream_id = 0;
  dma_engine.SetInputControllerStreamAddress(expected_stream_id,
                                             expected_address);
  int resulting_address =  // NOLINT
      dma_engine.GetInputControllerStreamAddress(expected_stream_id);
  return 0;
}