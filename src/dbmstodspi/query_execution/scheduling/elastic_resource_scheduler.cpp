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

#include "elastic_resource_scheduler.hpp"
#include "elastic_scheduling_graph_parser.hpp"

using orkhestrafs::dbmstodspi::ElasticResourceNodeScheduler;
using orkhestrafs::dbmstodspi::ElastiSchedulingGraphParser;

auto ElasticResourceNodeScheduler::FindAcceleratedQueryNodeSets(
    std::vector<std::shared_ptr<QueryNode>> starting_nodes,
    const std::map<ConfigurableModulesVector, std::string>&
        supported_accelerator_bitstreams,
    const std::map<QueryOperationType, std::vector<std::vector<int>>>&
        existing_modules_library,
    std::map<std::string,
             std::map<int, std::vector<std::pair<std::string, int>>>>&
        linked_nodes)
    -> std::queue<std::pair<ConfigurableModulesVector,
                            std::vector<std::shared_ptr<QueryNode>>>> {

  ElastiSchedulingGraphParser::PreprocessNodes(starting_nodes);

  std::vector<std::vector<std::pair<ConfigurableModulesVector,
                       std::vector<std::shared_ptr<QueryNode>>>>>
      all_plans;

  return plan_evaluator_->GetBestPlan(all_plans);
}