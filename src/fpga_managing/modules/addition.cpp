#include "addition.hpp"

using namespace dbmstodspi::fpga_managing::modules;

void dbmstodspi::fpga_managing::modules::Addition::DefineInput(int stream_id,
                                                               int chunk_id) {
  AccelerationModule::WriteToModule(0, (chunk_id << 8) + stream_id);
}

void dbmstodspi::fpga_managing::modules::Addition::SetInputSigns(
    std::bitset<8> is_value_negative) {
  AccelerationModule::WriteToModule(4, is_value_negative.to_ulong());
}

void dbmstodspi::fpga_managing::modules::Addition::SetLiteralValues(
    std::array<std::pair<uint32_t, uint32_t>, 8> literal_values) {
  for (int i = 0; i < literal_values.size(); i++) {
    AccelerationModule::WriteToModule(64 + (i * 8), literal_values.at(i).first);
    AccelerationModule::WriteToModule(64 + (i * 8) + 4,
                                      literal_values.at(i).second);
  }
}
