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

#include "elastic_scheduling_graph_parser.hpp"

#include "pre_scheduling_processor.hpp"

using orkhestrafs::dbmstodspi::ElastiSchedulingGraphParser;
using orkhestrafs::dbmstodspi::PreSchedulingProcessor;

void ElastiSchedulingGraphParser::PreprocessNodes(
    const std::vector<std::string>& available_nodes,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const std::vector<std::string>& processed_nodes,
    std::map<std::string, SchedulingQueryNode>& graph,
    std::map<std::string, TableMetadata>& data_tables,
    AcceleratorLibraryInterface& accelerator_library) {
  PreSchedulingProcessor::AddSatisfyingBitstreamLocationsToGraph(
      hw_library, graph, data_tables, accelerator_library, available_nodes,
      processed_nodes);
}

void ElastiSchedulingGraphParser::PlaceNodesRecursively(
    std::vector<std::string> starting_nodes,
    std::vector<std::string> processed_nodes,
    std::map<std::string, SchedulingQueryNode> graph, ScheduledRun current_run,
    std::vector<ScheduledRun> current_plan,
    std::map<std::vector<ScheduledRun>, ExecutionPlanSchedulingData>&
        resulting_plan,
    bool reduce_single_runs,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    int& min_runs, std::map<std::string, TableMetadata> data_tables,
    const std::vector<std::vector<ModuleSelection>>& heuristics,
    std::pair<int, int>& statistics_counters,
    const std::vector<std::string>& constrained_first_nodes,
    std::vector<std::string> blocked_nodes,
    std::vector<std::string> next_run_blocked_nodes) {}
