#include "node_scheduler.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
/// Good documentation for using matchers:
/// https://github.com/google/googletest/blob/master/docs/reference/matchers.md
namespace {

using dbmstodspi::query_managing::query_scheduling_data::
    NodeOperationParameters;
using ModulesCombo = dbmstodspi::query_managing::query_scheduling_data::
    ConfigurableModulesVector;
using Node = dbmstodspi::query_managing::query_scheduling_data::QueryNode;
using OpType = dbmstodspi::fpga_managing::operation_types::QueryOperationType;

const std::vector<std::string> kDefaultFileNames;
const std::vector<std::shared_ptr<Node>> kDefaultNextNodes;
const std::vector<std::weak_ptr<Node>> kDefaultPreviousNodes;
const NodeOperationParameters kDefaultParameters;

const int kSomeInteger = 2;

void CheckNodeFields(
    std::vector<std::string> input_files, std::vector<std::string> output_files,
    OpType operation, std::vector<std::shared_ptr<Node>> next,
    std::vector<std::weak_ptr<Node>> previous,
    dbmstodspi::query_managing::query_scheduling_data::NodeOperationParameters
        parameters,
    int location, Node comparable_node) {
  EXPECT_THAT(comparable_node,
              testing::Field("input_files", &Node::input_data_definition_files,
                             input_files));
  EXPECT_THAT(
      comparable_node,
      testing::Field("output_files", &Node::output_data_definition_files,
                     output_files));
  EXPECT_THAT(comparable_node,
              testing::Field("operation", &Node::operation_type, operation));
  EXPECT_THAT(comparable_node, testing::Field("next", &Node::next_nodes, next));
  EXPECT_THAT(
      comparable_node,
      testing::Field("parameters", &Node::operation_parameters, parameters));
  EXPECT_THAT(comparable_node,
              testing::Field("location", &Node::module_location, location));

  // EXPECT_THAT(comparable_node,
  //            testing::Field("previous", &Node::previous_nodes, previous));
  ASSERT_TRUE(std::equal(
      previous.begin(), previous.end(), comparable_node.previous_nodes.begin(),
      [](const std::weak_ptr<Node> l, const std::weak_ptr<Node> r) {
        return l.lock() == r.lock();
      }));
}

void CheckNodeEquality(Node real_node, Node expected_node) {
  CheckNodeFields(expected_node.input_data_definition_files,
                  expected_node.output_data_definition_files,
                  expected_node.operation_type, expected_node.next_nodes,
                  expected_node.previous_nodes,
                  expected_node.operation_parameters,
                  expected_node.module_location, real_node);
}

TEST(NodeSchedulerTest, MultipleAvailableNodesFindsCorrectNode) {
  auto filtering_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kFilter, kDefaultNextNodes,
      kDefaultPreviousNodes, kDefaultParameters);

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

  auto expected_node = *filtering_query;
  std::vector<std::shared_ptr<Node>> starting_nodes = {filtering_query};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, supported_bitstreams, existing_modules);

  expected_node.module_location = 1;

  ModulesCombo expected_module_vector = {{OpType::kFilter, {}}};

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_node);
}

TEST(NodeSchedulerTest, TwoNodesWereFoundWithDifferentRuns) {
  auto first_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kLinearSort,
      kDefaultNextNodes, kDefaultPreviousNodes, kDefaultParameters);
  auto second_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kLinearSort,
      kDefaultNextNodes, kDefaultPreviousNodes, kDefaultParameters);

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

  auto expected_first_node = *first_query;
  auto expected_second_node = *second_query;
  std::vector<std::shared_ptr<Node>> starting_nodes = {first_query,
                                                       second_query};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, supported_bitstreams, existing_modules);

  expected_first_node.module_location = 1;
  expected_second_node.module_location = 1;

  ModulesCombo expected_module_vector = {{OpType::kLinearSort, {}}};

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST(NodeSchedulerTest, TwoNodesWereFoundWithinTheSameRun) {
  auto first_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kLinearSort,
      kDefaultNextNodes, kDefaultPreviousNodes, kDefaultParameters);
  auto second_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kMergeSort,
      kDefaultNextNodes, kDefaultPreviousNodes, kDefaultParameters);

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

  auto expected_first_node = *first_query;
  auto expected_second_node = *second_query;
  std::vector<std::shared_ptr<Node>> starting_nodes = {first_query,
                                                       second_query};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, supported_bitstreams, existing_modules);

  expected_first_node.module_location = 1;
  expected_second_node.module_location = 2;

  ModulesCombo expected_module_vector = {{OpType::kLinearSort, {}},
                                         {OpType::kMergeSort, {}}};

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  CheckNodeEquality(*scheduling_results.front().second[1],
                    expected_second_node);
}

TEST(NodeSchedulerTest, PassthroughNodeIsUsedInTheSameRun) {
  auto first_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kPassThrough,
      kDefaultNextNodes, kDefaultPreviousNodes, kDefaultParameters);
  auto second_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kMergeSort,
      kDefaultNextNodes, kDefaultPreviousNodes, kDefaultParameters);

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

  auto expected_first_node = *first_query;
  auto expected_second_node = *second_query;
  std::vector<std::shared_ptr<Node>> starting_nodes = {first_query,
                                                       second_query};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, supported_bitstreams, existing_modules);

  expected_first_node.module_location = -1;
  expected_second_node.module_location = 1;

  ModulesCombo expected_module_vector = {{OpType::kMergeSort, {}}};

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[1], expected_first_node);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST(NodeSchedulerTest, TwoPipelinedNodesWereFoundWithDifferentRuns) {
  auto first_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kJoin, kDefaultNextNodes,
      kDefaultPreviousNodes, kDefaultParameters);
  auto second_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kMergeSort,
      kDefaultNextNodes, kDefaultPreviousNodes, kDefaultParameters);
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

  auto expected_first_node = *first_query;
  auto expected_second_node = *second_query;
  std::vector<std::shared_ptr<Node>> starting_nodes = {first_query,
                                                       second_query};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, supported_bitstreams, existing_modules);

  expected_first_node.module_location = 1;
  expected_first_node.next_nodes = {nullptr};
  expected_second_node.module_location = 1;
  expected_second_node.previous_nodes = {std::weak_ptr<Node>()};

  ModulesCombo expected_module_vector = {{OpType::kJoin, {}}};
  ModulesCombo expected_second_module_vector = {{OpType::kMergeSort, {}}};

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_second_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST(NodeSchedulerTest, TwoPipelinedNodesWereFoundWithinTheSameRun) {
  auto first_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kJoin, kDefaultNextNodes,
      kDefaultPreviousNodes, kDefaultParameters);
  auto second_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kMergeSort,
      kDefaultNextNodes, kDefaultPreviousNodes, kDefaultParameters);
  first_query->next_nodes = {second_query};
  second_query->previous_nodes = {first_query};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"},
      {{{OpType::kMergeSort, {}}, {OpType::kJoin, {}}}, "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  auto expected_first_node = *first_query;
  auto expected_second_node = *second_query;
  std::vector<std::shared_ptr<Node>> starting_nodes = {first_query,
                                                       second_query};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, supported_bitstreams, existing_modules);

  expected_first_node.module_location = 1;
  expected_second_node.module_location = 1;

  ModulesCombo expected_module_vector = {{OpType::kMergeSort, {}},
                                         {OpType::kJoin, {}}};

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  CheckNodeEquality(*scheduling_results.front().second[1],
                    expected_second_node);
}

}  // namespace