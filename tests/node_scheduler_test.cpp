#include "node_scheduler.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
namespace {

const int kSomeInteger = 2;

using ModulesCombo = dbmstodspi::query_managing::query_scheduling_data::
    ConfigurableModulesVector;
using Node = dbmstodspi::query_managing::query_scheduling_data::QueryNode;
using OpType = dbmstodspi::fpga_managing::operation_types::QueryOperation;

TEST(NodeSchedulerTest, ScheduleNodes) {
  Node filtering_query = {
      {"Some_input.csv"}, {"Some_output.csv"},
      OpType::kFilter,    {nullptr},
      {nullptr},          {{{}}, {{}, {kSomeInteger}}, {{0}}}};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{OpType::kFilter}, "Some_bitstream"}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, {filtering_query}, supported_bitstreams);

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push({{OpType::kFilter}, {filtering_query}});
  ASSERT_EQ(expected_results, scheduling_results);
}
}  // namespace