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

#include <memory>
#include <string>

#include "execution_plan_graph_interface.hpp"

using orkhestrafs::core_interfaces::ExecutionPlanGraphInterface;

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Factory for creating query plan graphs.
 */
class GraphCreatorInterface {
 public:
  virtual ~GraphCreatorInterface() = default;
  /**
   * @brief Read the given input_def file and make the query plan graph object.
   * @param graph_def_filename File containing the query plan information.
   * @return Query plan graph created form the given JSON.
   */
  virtual auto MakeGraph(std::string graph_def_filename)
      -> std::unique_ptr<ExecutionPlanGraphInterface> = 0;
};
}  // namespace orkhestrafs::dbmstodspi