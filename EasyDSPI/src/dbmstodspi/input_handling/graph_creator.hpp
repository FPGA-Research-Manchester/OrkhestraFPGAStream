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