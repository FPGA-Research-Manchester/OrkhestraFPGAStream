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

class NodeSchedulerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    base_parameters_ = {{{}}, {{}}, {{}}};

    base_supported_bitstreams_ = {
        {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
        {{{OpType::kJoin, {}}}, "Some_bitstream"},
        {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
        {{{OpType::kFilter, {}}}, "Some_bitstream"},
        {{{OpType::kPassThrough, {}}}, "Some_bitstream"}};
    base_existing_modules_ = {{OpType::kMergeSort, {}},
                              {OpType::kJoin, {}},
                              {OpType::kLinearSort, {}},
                              {OpType::kFilter, {}},
                              {OpType::kPassThrough, {}}};
    query_node_a_ = std::make_shared<Node>(
        kDefaultFileNames, kDefaultFileNames, OpType::kFilter,
        kDefaultNextNodes, kDefaultPreviousNodes, base_parameters_);
    query_node_b_ = std::make_shared<Node>(
        kDefaultFileNames, kDefaultFileNames, OpType::kLinearSort,
        kDefaultNextNodes, kDefaultPreviousNodes, base_parameters_);
    passthrough_node_ = std::make_shared<Node>(
        kDefaultFileNames, kDefaultFileNames, OpType::kPassThrough,
        kDefaultNextNodes, kDefaultPreviousNodes, base_parameters_);
  }

  std::map<ModulesCombo, std::string> base_supported_bitstreams_;
  std::map<OpType, std::vector<std::vector<int>>> base_existing_modules_;

  std::shared_ptr<Node> passthrough_node_;
  // Two base nodes with a different operation
  std::shared_ptr<Node> query_node_a_;
  std::shared_ptr<Node> query_node_b_;

  NodeOperationParameters base_parameters_;
};

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

TEST_F(NodeSchedulerTest, MultipleAvailableNodesFindsCorrectNode) {
  auto expected_node = *query_node_a_;
  expected_node.module_location = 1;
  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, base_supported_bitstreams_, base_existing_modules_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_node);
}

TEST_F(NodeSchedulerTest, TwoNodesWereFoundWithDifferentRuns) {
  auto first_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kLinearSort,
      kDefaultNextNodes, kDefaultPreviousNodes, base_parameters_);
  auto second_query = std::make_shared<Node>(
      kDefaultFileNames, kDefaultFileNames, OpType::kLinearSort,
      kDefaultNextNodes, kDefaultPreviousNodes, base_parameters_);

  auto expected_first_node = *first_query;
  auto expected_second_node = *second_query;
  expected_first_node.module_location = 1;
  expected_second_node.module_location = 1;
  ModulesCombo expected_module_vector = {{first_query->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {first_query,
                                                       second_query};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, base_supported_bitstreams_, base_existing_modules_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, TwoNodesWereFoundWithinTheSameRun) {
  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}},
                                         {query_node_b_->operation_type, {}}};

  base_supported_bitstreams_.insert({expected_module_vector, "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  expected_second_node.module_location = 2;
  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_,
                                                       query_node_b_};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, base_supported_bitstreams_, base_existing_modules_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  CheckNodeEquality(*scheduling_results.front().second[1],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, PassthroughNodeIsUsedInTheSameRun) {
  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *passthrough_node_;
  expected_first_node.module_location = 1;
  expected_second_node.module_location = -1;

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_,
                                                       passthrough_node_};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, base_supported_bitstreams_, base_existing_modules_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  CheckNodeEquality(*scheduling_results.front().second[1],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, TwoPipelinedNodesWereFoundWithDifferentRuns) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  expected_first_node.next_nodes = {nullptr};
  expected_second_node.module_location = 1;
  expected_second_node.previous_nodes = {std::weak_ptr<Node>()};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};
  ModulesCombo expected_second_module_vector = {
      {query_node_b_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, base_supported_bitstreams_, base_existing_modules_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_second_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, TwoPipelinedNodesWereFoundWithinTheSameRun) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}},
                                         {query_node_b_->operation_type, {}}};

  base_supported_bitstreams_.insert({expected_module_vector, "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  expected_second_node.module_location = 2;

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, base_supported_bitstreams_, base_existing_modules_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  CheckNodeEquality(*scheduling_results.front().second[1],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, DifferentRunsBecauseOfInputProjection) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  query_node_b_->operation_parameters.input_stream_parameters = {
      {0, 1, 2, 3, 3}};

  base_supported_bitstreams_.insert({{{query_node_a_->operation_type, {}},
                                      {query_node_b_->operation_type, {}}},
                                     "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  expected_first_node.next_nodes = {nullptr};
  expected_second_node.module_location = 1;
  expected_second_node.previous_nodes = {std::weak_ptr<Node>()};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};
  ModulesCombo expected_second_module_vector = {
      {query_node_b_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, base_supported_bitstreams_, base_existing_modules_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_second_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, DifferentRunsBecauseOfOutputProjection) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  query_node_a_->operation_parameters.output_stream_parameters = {
      {0, 1, 2, 3, 3}};

  base_supported_bitstreams_.insert({{{query_node_a_->operation_type, {}},
                                      {query_node_b_->operation_type, {}}},
                                     "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  expected_first_node.next_nodes = {nullptr};
  expected_second_node.module_location = 1;
  expected_second_node.previous_nodes = {std::weak_ptr<Node>()};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};
  ModulesCombo expected_second_module_vector = {
      {query_node_b_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, base_supported_bitstreams_, base_existing_modules_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_second_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, DifferentRunsBecauseOfIOProjection) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  query_node_a_->operation_parameters.output_stream_parameters = {
      {0, 1, 2, 3, 3}};
  query_node_b_->operation_parameters.input_stream_parameters = {
      {0, 1, 2, 3, 3}};

  base_supported_bitstreams_.insert({{{query_node_a_->operation_type, {}},
                                      {query_node_b_->operation_type, {}}},
                                     "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  expected_first_node.next_nodes = {nullptr};
  expected_second_node.module_location = 1;
  expected_second_node.previous_nodes = {std::weak_ptr<Node>()};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};
  ModulesCombo expected_second_module_vector = {
      {query_node_b_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, base_supported_bitstreams_, base_existing_modules_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_second_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, DifferentRunsBecauseOfSecondOutputProjection) {
  query_node_a_->next_nodes = {nullptr, query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  std::vector<int> some_data = {0, 1, 2, 3, 3};

  query_node_a_->operation_parameters.output_stream_parameters = {
      {},        some_data, some_data, some_data,
      some_data, some_data, some_data, some_data};

  base_supported_bitstreams_.insert({{{query_node_a_->operation_type, {}},
                                      {query_node_b_->operation_type, {}}},
                                     "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  expected_first_node.next_nodes = {nullptr, nullptr};
  expected_second_node.module_location = 1;
  expected_second_node.previous_nodes = {std::weak_ptr<Node>()};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};
  ModulesCombo expected_second_module_vector = {
      {query_node_b_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, base_supported_bitstreams_, base_existing_modules_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_second_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, SameRunsDespiteIOProjection) {
  query_node_a_->next_nodes = {nullptr, query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  std::vector<int> some_data = {0, 1, 2, 3, 3};

  query_node_a_->operation_parameters.output_stream_parameters = {
      some_data, some_data, some_data, some_data,
      {},        some_data, some_data, some_data};
  query_node_a_->operation_parameters.input_stream_parameters = {
      {0, 1, 2, 3, 3}};
  query_node_b_->operation_parameters.output_stream_parameters = {
      {1, 2, 3, 4, 5, 6, 6}};
  query_node_b_->operation_parameters.input_stream_parameters = {
      {}, {12}, {1, 2, 3, 4}};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}},
                                         {query_node_b_->operation_type, {}}};

  base_supported_bitstreams_.insert({expected_module_vector, "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  expected_second_node.module_location = 2;

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results =
      dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
          starting_nodes, base_supported_bitstreams_, base_existing_modules_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  CheckNodeEquality(*scheduling_results.front().second[1],
                    expected_second_node);
}

}  // namespace