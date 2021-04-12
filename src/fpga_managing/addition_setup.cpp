#include "addition_setup.hpp"

using namespace dbmstodspi::fpga_managing;

void dbmstodspi::fpga_managing::AdditionSetup::SetupAdditionModule(
    modules::AdditionInterface& addition_module, int stream_id) {
  // Hard coded for now!
  addition_module.DefineInput(stream_id, 0);

  std::array<bool, 8> is_value_negative;
  is_value_negative.at(6) = true;

  addition_module.SetInputSigns(is_value_negative);
  std::array<std::pair<uint32_t, uint32_t>, 8> literal_values;
  literal_values.fill(std::make_pair(0, 0));
  literal_values.at(6).first = 100;
  addition_module.SetLiteralValues(literal_values);
}
