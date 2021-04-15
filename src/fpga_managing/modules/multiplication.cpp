#include "multiplication.hpp"

using namespace dbmstodspi::fpga_managing::modules;

void dbmstodspi::fpga_managing::modules::Multiplication::DefineActiveStreams(
    std::bitset<16> active_streams) {
  AccelerationModule::WriteToModule(0, active_streams.to_ulong());
}

void dbmstodspi::fpga_managing::modules::Multiplication::
    ChooseMultiplicationResults(int chunk_id, std::bitset<8> active_elements) {
  AccelerationModule::WriteToModule(128 + chunk_id * 4,
                                    active_elements.to_ulong());
}
