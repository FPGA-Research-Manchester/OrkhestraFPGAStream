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

#pragma once

#include <map>
#include <string>
#include <utility>

#include "state_interface.hpp"

using orkhestrafs::dbmstodspi::GraphProcessingFSMInterface;

namespace orkhestrafs::dbmstodspi {
/**
 * @brief State for setting up nodes.
 */
class InteractiveState : public StateInterface {
 private:
  void PrintOptions(GraphProcessingFSMInterface* fsm);
  auto GetOption(GraphProcessingFSMInterface* fsm) -> int;
  auto GetStdInput() -> std::string;
  auto GetInteger() -> int;
  auto GetDouble() -> double;
  auto GetExecutionPlanFile(const std::pair<std::string, std::string>& input)
      -> std::string;
  void PrintOutGivenOptions(const std::vector<std::string> list_of_options);
  auto GetBitstreamToLoad(
      const std::map<QueryOperationType, OperationPRModules> bitstream_map)
      -> ScheduledModule;
  auto GetQueryFromInput() -> std::pair<std::string, std::string>;
  std::map<QueryOperationType, std::string> operation_names_ = {
      {QueryOperationType::kAddition, "Addition"},
      {QueryOperationType::kAggregationSum, "Aggregation sum"},
      {QueryOperationType::kFilter, "Filter"},
      {QueryOperationType::kJoin, "Join"},
      {QueryOperationType::kLinearSort, "Linear sorter"},
      {QueryOperationType::kMergeSort, "Merge sorter"},
      {QueryOperationType::kMultiplication, "Multiplication"},
      {QueryOperationType::kSobel, "Sobel"},
      {QueryOperationType::kBlackWhite, "Convert to Monochrome"},
  };

 public:
  ~InteractiveState() override = default;

  auto Execute(GraphProcessingFSMInterface* fsm)
      -> std::unique_ptr<StateInterface> override;
};
}  // namespace orkhestrafs::dbmstodspi