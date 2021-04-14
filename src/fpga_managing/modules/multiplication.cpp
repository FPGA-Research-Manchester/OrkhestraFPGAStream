#include "multiplication.hpp"

using namespace dbmstodspi::fpga_managing::modules;

void dbmstodspi::fpga_managing::modules::Multiplication::DefineActiveStreams(
    std::array<bool, 16> active_streams) {
  int active_streams_one_hot = 0;
  for (int i = 0; i < active_streams.size(); i++) {
    active_streams_one_hot += active_streams[i] << i;
  }
  AccelerationModule::WriteToModule(0,
                                    active_streams_one_hot);
}

void dbmstodspi::fpga_managing::modules::Multiplication::ChooseMultiplicationResults(
    int chunk_id, std::array<bool, 8> active_elements) {
  int active_elements_one_hot = 0;
  for (int i = 0; i < active_elements.size(); i++) {
    active_elements_one_hot += active_elements[i] << i;
  }
  AccelerationModule::WriteToModule(128 + chunk_id * 4,
                                    active_elements_one_hot);
}
