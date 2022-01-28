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

#include "addition.hpp"
#include "addition_interface.hpp"
#include "logger.hpp"

using orkhestrafs::dbmstodspi::AdditionSetup;

using orkhestrafs::dbmstodspi::Addition;
using orkhestrafs::dbmstodspi::AdditionInterface;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

void AdditionSetup::SetupModule(AccelerationModule& acceleration_module,
                                const AcceleratedQueryNode& module_parameters) {
  Log(LogLevel::kInfo,
      "Configuring addition on pos " +
          std::to_string(module_parameters.operation_module_location));
  if (module_parameters.input_streams[0].stream_id != 15) {
    AdditionSetup::SetupAdditionModule(
        dynamic_cast<AdditionInterface&>(acceleration_module),
        module_parameters.input_streams[0].stream_id,
        module_parameters.operation_parameters);
  } else {
    AdditionSetup::SetupPassthroughAddition(
        dynamic_cast<AdditionInterface&>(acceleration_module));
  }

}

void AdditionSetup::SetupPassthroughAddition(
    AdditionInterface& addition_module) {
  addition_module.DefineInput(15, 0);
}

auto AdditionSetup::CreateModule(MemoryManagerInterface* memory_manager,
                                 int module_postion)
    -> std::unique_ptr<AccelerationModule> {
  return std::make_unique<Addition>(memory_manager, module_postion);
}

void AdditionSetup::SetupAdditionModule(
    AdditionInterface& addition_module, int stream_id,
    const std::vector<std::vector<int>>& operation_parameters) {
  addition_module.DefineInput(stream_id, operation_parameters.at(0).at(0));
  addition_module.SetInputSigns(
      ReverseNegationSpecification(operation_parameters.at(1)));
  addition_module.SetLiteralValues(
      ReverseLiteralValues(operation_parameters.at(2)));
}

// Assuming input vectors are correct size!
auto AdditionSetup::ReverseLiteralValues(std::vector<int> input_constant_values)
    -> std::array<std::pair<uint32_t, uint32_t>, 8> {
  std::array<std::pair<uint32_t, uint32_t>, 8> literal_values;
  for (int i = literal_values.size() - 1; i >= 0; i--) {
    literal_values.at(7 - i) = {input_constant_values.at(i * 2 + 1),
                                input_constant_values.at(i * 2)};
  }
  return literal_values;
}
auto AdditionSetup::ReverseNegationSpecification(
    std::vector<int> negation_specification) -> std::bitset<8> {
  std::bitset<8> negation_required;
  for (int i = negation_required.size() - 1; i >= 0; i--) {
    negation_required.set(7 - i, negation_specification.at(i) != 0);
  }
  return negation_required;
}
