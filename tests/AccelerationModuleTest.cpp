#include <gtest/gtest.h>

#include "MockAccelerationModule.hpp"
namespace {
const int kDefaultValue = -1;

class AccelerationModuleTest : public testing::Test {
 protected:
  void SetUp() override {
    memoryPointer_ = new int[2097152];
    for (int i = 0; i < 2097152; i++) {
      memoryPointer_[i] = kDefaultValue;
    }
  }
  void TearDown() override { delete[] memoryPointer_; }
  int* volatile memoryPointer_ = nullptr;
};

TEST_F(AccelerationModuleTest, WriteToModule) {
  MockAccelerationModule mock_module(memoryPointer_, 0);

  EXPECT_EQ(kDefaultValue, memoryPointer_[0]);
  mock_module.writeToModule(0, 0);
  EXPECT_EQ(0, memoryPointer_[0]);

  EXPECT_EQ(kDefaultValue, memoryPointer_[1]);
  mock_module.writeToModule(1, 10);
  EXPECT_EQ(10, memoryPointer_[1]);

  MockAccelerationModule second_mock_module(memoryPointer_, 1);

  EXPECT_EQ(kDefaultValue, memoryPointer_[1048576]);
  second_mock_module.writeToModule(0, 1);
  EXPECT_EQ(1, memoryPointer_[1048576]);
  second_mock_module.writeToModule(0, 11);
  EXPECT_EQ(11, memoryPointer_[1048576]);
}

TEST_F(AccelerationModuleTest, ReadFromModule) {
  MockAccelerationModule mock_module(memoryPointer_, 0);

  EXPECT_EQ(kDefaultValue, mock_module.readFromModule(2));
  memoryPointer_[2] = 100;
  EXPECT_EQ(100, mock_module.readFromModule(2));

  MockAccelerationModule second_mock_module(memoryPointer_, 1);

  EXPECT_EQ(kDefaultValue, second_mock_module.readFromModule(2));
  memoryPointer_[1048578] = 101;
  EXPECT_EQ(101, second_mock_module.readFromModule(2));
}
}  // namespace