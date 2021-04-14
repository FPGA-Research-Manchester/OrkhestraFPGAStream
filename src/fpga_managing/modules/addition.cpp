#include "addition.hpp"

using namespace dbmstodspi::fpga_managing::modules;

void dbmstodspi::fpga_managing::modules::Addition::DefineInput(int stream_id,
                                                               int chunk_id) {
  AccelerationModule::WriteToModule(0, (chunk_id << 8) + stream_id);
}

void dbmstodspi::fpga_managing::modules::Addition::SetInputSigns(
    std::array<bool, 8> is_value_negative) {
  AccelerationModule::WriteToModule(
      4, (is_value_negative.at(7) << 7) + (is_value_negative.at(6) << 6) +
             (is_value_negative.at(5) << 5) + (is_value_negative.at(4) << 4) +
             (is_value_negative.at(3) << 3) + (is_value_negative.at(2) << 2) +
             (is_value_negative.at(1) << 1) + is_value_negative.at(0));
}

void dbmstodspi::fpga_managing::modules::Addition::SetLiteralValues(
    std::array<std::pair<uint32_t, uint32_t>, 8> literal_values) {
  for (int i = 0; i < literal_values.size(); i++) {
    AccelerationModule::WriteToModule(64 + (i * 8),
                                      literal_values.at(i).first);
    AccelerationModule::WriteToModule(64 + (i * 8) + 4,
                                      literal_values.at(i).second);
  }
}
