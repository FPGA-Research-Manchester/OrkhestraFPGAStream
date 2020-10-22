#include <gtest/gtest.h>

#include <limits.h>

#include "mock_acceleration_module.hpp"
namespace {
const int kDefaultValue = -1;

// Disabled for now due to missing mock acceleration instances needed for acceleration modules.
/*
TEST(AccelerationModuleTest, WriteToModule) {
  std::vector<uint32_t> memory_pointer(524288, kDefaultValue);

  MockAccelerationModule mock_module(memory_pointer.data(), 0);

  EXPECT_EQ(UINT_MAX, memory_pointer[0]);
  mock_module.WriteToModule(0, 0);
  EXPECT_EQ(0, memory_pointer[0]);

  EXPECT_EQ(UINT_MAX, memory_pointer[1]);
  mock_module.WriteToModule(1 * sizeof(uint32_t), 10);
  EXPECT_EQ(10, memory_pointer[1]);

  MockAccelerationModule second_mock_module(memory_pointer.data(), 1);

  EXPECT_EQ(UINT_MAX, memory_pointer[262144]);
  second_mock_module.WriteToModule(0, 1);
  EXPECT_EQ(1, memory_pointer[262144]);
  second_mock_module.WriteToModule(0, 11);
  EXPECT_EQ(11, memory_pointer[262144]);
}

TEST(AccelerationModuleTest, ReadFromModule) {
  std::vector<uint32_t> memory_pointer(524288, kDefaultValue);

  MockAccelerationModule mock_module(memory_pointer.data(), 0);

  EXPECT_EQ(kDefaultValue, mock_module.ReadFromModule(2 * sizeof(uint32_t)));
  memory_pointer[2] = 100;
  EXPECT_EQ(100, mock_module.ReadFromModule(2 * sizeof(uint32_t)));

  MockAccelerationModule second_mock_module(memory_pointer.data(), 1);

  EXPECT_EQ(kDefaultValue,
            second_mock_module.ReadFromModule(2 * sizeof(uint32_t)));
  memory_pointer[262146] = 101;
  EXPECT_EQ(101, second_mock_module.ReadFromModule(2 * sizeof(uint32_t)));
}
*/
}  // namespace