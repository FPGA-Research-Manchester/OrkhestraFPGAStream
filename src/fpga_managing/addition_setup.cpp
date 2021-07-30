/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "addition_setup.hpp"

using namespace dbmstodspi::fpga_managing;

void dbmstodspi::fpga_managing::AdditionSetup::SetupAdditionModule(
    modules::AdditionInterface& addition_module, int stream_id,
    const std::vector<std::vector<int>>& operation_parameters) {
  addition_module.DefineInput(stream_id, operation_parameters.at(0).at(0));
  addition_module.SetInputSigns(
      ReverseNegationSpecification(operation_parameters.at(1)));
  addition_module.SetLiteralValues(
      ReverseLiteralValues(operation_parameters.at(2)));
}

// Assuming input vectors are correct size!
auto dbmstodspi::fpga_managing::AdditionSetup::ReverseLiteralValues(
    std::vector<int> input_constant_values)
    -> std::array<std::pair<uint32_t, uint32_t>, 8> {
  std::array<std::pair<uint32_t, uint32_t>, 8> literal_values;
  for (int i = literal_values.size() - 1; i >= 0; i--) {
    literal_values.at(7 - i) = {input_constant_values.at(i * 2 + 1),
                                input_constant_values.at(i * 2)};
  }
  return literal_values;
}
auto dbmstodspi::fpga_managing::AdditionSetup::ReverseNegationSpecification(
    std::vector<int> negation_specification) -> std::bitset<8> {
  std::bitset<8> negation_required;
  for (int i = negation_required.size() - 1; i >= 0; i--) {
    negation_required.set(7 - i, negation_specification.at(i) != 0);
  }
  return negation_required;
}
