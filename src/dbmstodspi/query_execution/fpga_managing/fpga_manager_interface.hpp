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
#include <array>
#include <vector>
#include <map>

#include "accelerated_query_node.hpp"
#include "query_acceleration_constants.hpp"

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Implemented by #FPGAManager
 */
class FPGAManagerInterface {
 public:
  virtual ~FPGAManagerInterface() = default;
  virtual void SetupQueryAcceleration(
      const std::vector<AcceleratedQueryNode>& query_nodes) = 0;

  virtual auto RunQueryAcceleration(int timeout, std::map<int, std::vector<double>>& read_back_values)
      -> std::array<int, query_acceleration_constants::kMaxIOStreamCount> = 0;
};

}  // namespace orkhestrafs::dbmstodspi