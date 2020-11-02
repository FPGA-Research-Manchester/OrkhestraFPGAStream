#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <limits.h>

#include "mock_memory_manager.hpp"
#include "mock_acceleration_module.hpp"
namespace {
const int kDefaultValue = -1;

TEST(AccelerationModuleTest, WriteToModule) {
  std::vector<uint32_t> memory_pointer(524288, kDefaultValue);
  MockMemoryManager mock_memory_manager;
  MockAccelerationModule mock_module(&mock_memory_manager, 0);

  EXPECT_CALL(mock_memory_manager, GetVirtualRegisterAddress(0))
      .Times(1)
      .WillOnce(::testing::Return(&memory_pointer[0]));

  EXPECT_EQ(UINT_MAX, memory_pointer[0]);
  mock_module.WriteToModule(0, 0);
  EXPECT_EQ(0, memory_pointer[0]);

  EXPECT_CALL(mock_memory_manager, GetVirtualRegisterAddress(1))
      .Times(1)
      .WillOnce(::testing::Return(&memory_pointer[1]));

  EXPECT_EQ(UINT_MAX, memory_pointer[1]);
  mock_module.WriteToModule(1, 10);
  EXPECT_EQ(10, memory_pointer[1]);

  MockAccelerationModule second_mock_module(&mock_memory_manager, 1);

  EXPECT_CALL(mock_memory_manager, GetVirtualRegisterAddress(1024 * 1024))
      .Times(2)
      .WillRepeatedly(::testing::Return(&memory_pointer[2]));

  EXPECT_EQ(UINT_MAX, memory_pointer[2]);
  second_mock_module.WriteToModule(0, 1);
  EXPECT_EQ(1, memory_pointer[2]);

  second_mock_module.WriteToModule(0, 11);
  EXPECT_EQ(11, memory_pointer[2]);
}

TEST(AccelerationModuleTest, ReadFromModule) {
  std::vector<uint32_t> memory_pointer(524288, kDefaultValue);
  MockMemoryManager mock_memory_manager;
  MockAccelerationModule mock_module(&mock_memory_manager, 0);

  EXPECT_CALL(mock_memory_manager, GetVirtualRegisterAddress(2))
      .Times(2)
      .WillRepeatedly(::testing::Return(&memory_pointer[0]));

  EXPECT_EQ(kDefaultValue, mock_module.ReadFromModule(2));
  memory_pointer[0] = 100;
  EXPECT_EQ(100, mock_module.ReadFromModule(2));

  MockAccelerationModule second_mock_module(&mock_memory_manager, 1);

  EXPECT_CALL(mock_memory_manager, GetVirtualRegisterAddress(1024 * 1024 + 2))
      .Times(2)
      .WillRepeatedly(::testing::Return(&memory_pointer[1]));

  EXPECT_EQ(kDefaultValue,
            second_mock_module.ReadFromModule(2));
  memory_pointer[1] = 101;
  EXPECT_EQ(101, second_mock_module.ReadFromModule(2));
}
}  // namespace