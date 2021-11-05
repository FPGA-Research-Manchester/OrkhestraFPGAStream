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

using orkhestrafs::dbmstodspi::ElastiSchedulingGraphParser;

void ElastiSchedulingGraphParser::PreprocessNodes(
    std::vector<std::shared_ptr<QueryNode>>& available_nodes) {
  // Do nothing

  // Add pre_scheduling_processor step here but first of all get all the
  // necessary data connected to here.
}
