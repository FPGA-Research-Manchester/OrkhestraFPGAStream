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

#include "gmock/gmock.h"
#include "node_scheduler_interface.hpp"

using orkhestrafs::dbmstodspi::NodeSchedulerInterface;

class MockNodeScheduler : public NodeSchedulerInterface {
 private:
  using NodeVector = std::vector<std::shared_ptr<QueryNode>>;
  using ResultingPlanQueue =
      std::queue<std::pair<ConfigurableModulesVector, NodeVector>>;
  using LinkedNodesMap =
      std::map<std::string,
               std::map<int, std::vector<std::pair<std::string, int>>>>;
  using AcceleratorMap = std::map<ConfigurableModulesVector, std::string>;
  using LinkedNodesMap =
      std::map<std::string,
               std::map<int, std::vector<std::pair<std::string, int>>>>;
  using ModulesMap =
      std::map<QueryOperationType, std::vector<std::vector<int>>>;

 public:
  MOCK_METHOD(ResultingPlanQueue, FindAcceleratedQueryNodeSets,
              (NodeVector starting_nodes,
               const AcceleratorMap& supported_accelerator_bitstreams,
               const ModulesMap& existing_modules_library,
               LinkedNodesMap& linked_nodes),
              (override));
};