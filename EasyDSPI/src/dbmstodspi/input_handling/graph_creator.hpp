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

#include "graph_creator_interface.hpp"
#include "json_reader_interface.hpp"

using easydspi::core_interfaces::ExecutionPlanGraphInterface;

namespace easydspi::dbmstodspi {
class GraphCreator : public GraphCreatorInterface {
 private:
  std::unique_ptr<JSONReaderInterface> json_reader_;

 public:
  ~GraphCreator() override = default;
  GraphCreator(std::unique_ptr<JSONReaderInterface> json_reader)
      : json_reader_{std::move(json_reader)} {};
  std::unique_ptr<ExecutionPlanGraphInterface> makeGraph(
      std::string graph_def_filename) override;
};
}  // namespace easydspi::dbmstodspi