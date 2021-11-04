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
#include "query_scheduling_data.hpp"

using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to contain static helper methods.
 */
class QuerySchedulingHelper {
 public:

  /**
   * @brief Find the index of the current node of the previous node's next nodes.
   * @param current_node Current node which's index is unknown
   * @param previous_node Previous node.
   * @return Integer showing which stream of the previous node is used for this node.
  */
  static auto FindNodePtrIndex(QueryNode* current_node, QueryNode* previous_node)
      -> int;
};

}  // namespace orkhestrafs::dbmstodspi