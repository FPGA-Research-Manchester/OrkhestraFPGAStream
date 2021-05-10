#include "node_scheduler.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
namespace {

const int kSomeInteger = 2;

using ModulesCombo = dbmstodspi::query_managing::query_scheduling_data::
    ConfigurableModulesVector;
using Node = dbmstodspi::query_managing::query_scheduling_data::QueryNode;
using OpType = dbmstodspi::fpga_managing::operation_types::QueryOperationType;

TEST(NodeSchedulerTest, MultipleAvailableNodesFindsCorrectNode) {
  Node filtering_query = {
      {"Some_input.csv"}, {"Some_output.csv"},
      OpType::kFilter,    {nullptr},
      {nullptr},          {{{}}, {{}, {kSomeInteger}}, {{}}}};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, {filtering_query}, supported_bitstreams,
      existing_modules);

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push({{{OpType::kFilter, {}}}, {filtering_query}});
  ASSERT_EQ(expected_results, scheduling_results);
}

TEST(NodeSchedulerTest, TwoNodesWereFoundWithDifferentRuns) {
  Node first_query = {{"Some_input.csv"},  {"Some_output.csv"},
                      OpType::kLinearSort, {nullptr},
                      {nullptr},           {{{}}, {{}, {kSomeInteger}}, {{}}}};
  Node second_query = {{"Some_input.csv"},  {"Some_output.csv"},
                       OpType::kLinearSort, {nullptr},
                       {nullptr},           {{{}}, {{}, {kSomeInteger}}, {{}}}};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, {first_query, second_query}, supported_bitstreams,
      existing_modules);

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push({{{OpType::kLinearSort, {}}}, {first_query}});
  expected_results.push({{{OpType::kLinearSort, {}}}, {second_query}});
  ASSERT_EQ(expected_results, scheduling_results);
}

TEST(NodeSchedulerTest, TwoNodesWereFoundWithinTheSameRun) {
  Node first_query = {{"Some_input.csv"},  {"Some_output.csv"},
                      OpType::kLinearSort, {nullptr},
                      {nullptr},           {{{}}, {{}, {kSomeInteger}}, {{}}}};
  Node second_query = {{"Some_input.csv"}, {"Some_output.csv"},
                       OpType::kMergeSort, {nullptr},
                       {nullptr},          {{{}}, {{}, {kSomeInteger}}, {{}}}};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}, {OpType::kMergeSort, {}}},
       "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, {first_query, second_query}, supported_bitstreams,
      existing_modules);

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push({{{OpType::kLinearSort, {}}, {OpType::kMergeSort, {}}},
                         {first_query, second_query}});
  ASSERT_EQ(expected_results, scheduling_results);
}

TEST(NodeSchedulerTest, PassthroughNodeIsUsedInTheSameRun) {
  Node first_query = {{"Some_input.csv"},
                      {"Some_output.csv"},
                      OpType::kPassThrough,
                      {nullptr},
                      {nullptr},
                      {{{}}, {{}, {kSomeInteger}}, {{}}}};
  Node second_query = {{"Some_input.csv"}, {"Some_output.csv"},
                       OpType::kMergeSort, {nullptr},
                       {nullptr},          {{{}}, {{}, {kSomeInteger}}, {{}}}};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, {first_query, second_query}, supported_bitstreams,
      existing_modules);

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push(
      {{{OpType::kMergeSort, {}}}, {second_query, first_query}});
  ASSERT_EQ(expected_results, scheduling_results);
}

TEST(NodeSchedulerTest, TwoPipelinedNodesWereFoundWithDifferentRuns) {
  auto first_query = std::make_shared<Node>(
      Node{{"Some_input.csv"},
           {"Some_output.csv"},
           OpType::kJoin,
           {nullptr},
           {nullptr},
           {{{}}, {{}, {kSomeInteger}}, {{kSomeInteger}}}});
  auto second_query = std::make_shared<Node>(
      Node{{"Some_input.csv"},
           {"Some_output.csv"},
           OpType::kMergeSort,
           {nullptr},
           {nullptr},
           {{{}}, {{}, {kSomeInteger}}, {{kSomeInteger}}}});
  first_query->next_nodes = {second_query};
  second_query->previous_nodes = {first_query};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, {*first_query}, supported_bitstreams,
      existing_modules);

  auto expected_first_query = *first_query;
  auto expected_second_query = *second_query;
  expected_first_query.next_nodes = {nullptr};
  expected_second_query.previous_nodes = {nullptr};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push({{{OpType::kJoin, {}}}, {expected_first_query}});
  expected_results.push({{{OpType::kMergeSort, {}}}, {expected_second_query}});
  ASSERT_EQ(expected_results, scheduling_results);
}

TEST(NodeSchedulerTest, TwoPipelinedNodesWereFoundWithinTheSameRun) {
  auto first_query = std::make_shared<Node>(
      Node{{"Some_input.csv"},
           {"Some_output.csv"},
           OpType::kJoin,
           {nullptr},
           {nullptr},
           {{{}}, {{}, {kSomeInteger}}, {{kSomeInteger}}}});
  auto second_query = std::make_shared<Node>(
      Node{{"Some_input.csv"},
           {"Some_output.csv"},
           OpType::kMergeSort,
           {nullptr},
           {nullptr},
           {{{}}, {{}, {kSomeInteger}}, {{kSomeInteger}}}});
  first_query->next_nodes = {second_query};
  second_query->previous_nodes = {first_query};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}, {OpType::kMergeSort, {}}}, "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, {*first_query}, supported_bitstreams,
      existing_modules);

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push({{{OpType::kJoin, {}}, {OpType::kMergeSort, {}}},
                         {*first_query, *second_query}});
  ASSERT_EQ(expected_results, scheduling_results);
}

}  // namespace