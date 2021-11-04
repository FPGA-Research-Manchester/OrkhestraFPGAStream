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

#include "query_scheduling_helper.hpp"

#include <stdexcept>

using orkhestrafs::dbmstodspi::QuerySchedulingHelper;

auto QuerySchedulingHelper::FindNodePtrIndex(QueryNode* current_node,
                                             QueryNode* previous_node) -> int {
  int index = -1;
  for (const auto& potential_current_node : previous_node->next_nodes) {
    int counter = 0;
    if (potential_current_node.get() == current_node) {
      if (index != -1) {
        throw std::runtime_error(
            "Currently can't support the same module taking multiple inputs "
            "from another module!");
      }
      index = counter;
    }
  }
  if (index == -1) {
    throw std::runtime_error(
        "No current node found!");
  }
  return index;
}