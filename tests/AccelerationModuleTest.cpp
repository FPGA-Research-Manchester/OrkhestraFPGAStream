#include <gtest/gtest.h>
#include "MockAccelerationModule.hpp"
namespace {
	const int DEFAULT_VALUE = -1;

	class AccelerationModuleTest : public testing::Test {
	protected:
		void SetUp() override {
			memoryPointer = new int[2097152];
			for (int i = 0; i < 2097152; i++) {
				memoryPointer[i] = DEFAULT_VALUE;
			}
		}
		void TearDown() override {
			delete[] memoryPointer;
		}
		int* volatile memoryPointer = nullptr;
	};

	TEST_F(AccelerationModuleTest, WriteToModule) {
		MockAccelerationModule mockModule(memoryPointer, 0);

		EXPECT_EQ(DEFAULT_VALUE, memoryPointer[0]);
		mockModule.writeToModule(0, 0);
		EXPECT_EQ(0, memoryPointer[0]);

		EXPECT_EQ(DEFAULT_VALUE, memoryPointer[1]);
		mockModule.writeToModule(1, 10);
		EXPECT_EQ(10, memoryPointer[1]);

		MockAccelerationModule secondMockModule(memoryPointer, 1);

		EXPECT_EQ(DEFAULT_VALUE, memoryPointer[1048576]);
		secondMockModule.writeToModule(0, 1);
		EXPECT_EQ(1, memoryPointer[1048576]);
		secondMockModule.writeToModule(0, 11);
		EXPECT_EQ(11, memoryPointer[1048576]);
	}

	TEST_F(AccelerationModuleTest, ReadFromModule) {
		MockAccelerationModule mockModule(memoryPointer, 0);

		EXPECT_EQ(DEFAULT_VALUE, mockModule.readFromModule(2));
		memoryPointer[2] = 100;
		EXPECT_EQ(100, mockModule.readFromModule(2));

		MockAccelerationModule secondMockModule(memoryPointer, 1);

		EXPECT_EQ(DEFAULT_VALUE, secondMockModule.readFromModule(2));
		memoryPointer[1048578] = 101;
		EXPECT_EQ(101, secondMockModule.readFromModule(2));
	}
}  // namespace