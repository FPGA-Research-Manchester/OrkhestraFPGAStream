#include <iostream>

#include <gtest/gtest.h>
#include "AccelerationModuleMock.hpp"

TEST(AccelerationModuleTest, testCase) {
	const int defaultValue = -1;

	int* volatile memoryPointer = new int[2097152];
	for (int i = 0; i < 2097152; i++) {
		memoryPointer[i] = defaultValue;
	}

	AccelerationModuleMock mockModule(memoryPointer, 0);

	EXPECT_EQ(defaultValue, memoryPointer[0]);
	mockModule.writeToModule(0, 0);
	EXPECT_EQ(0, memoryPointer[0]);

	EXPECT_EQ(defaultValue, memoryPointer[1]);
	mockModule.writeToModule(1, 10);
	EXPECT_EQ(10, memoryPointer[1]);

	EXPECT_EQ(defaultValue, mockModule.readFromModule(2));
	memoryPointer[2] = 100;
	EXPECT_EQ(100, mockModule.readFromModule(2));

	AccelerationModuleMock secondMockModule(memoryPointer, 1);

	EXPECT_EQ(defaultValue, memoryPointer[1048576]);
	secondMockModule.writeToModule(0, 1);
	EXPECT_EQ(1, memoryPointer[1048576]);

	EXPECT_EQ(defaultValue, memoryPointer[1048577]);
	secondMockModule.writeToModule(1, 11);
	EXPECT_EQ(11, memoryPointer[1048577]);

	EXPECT_EQ(defaultValue, secondMockModule.readFromModule(2));
	memoryPointer[1048578] = 101;
	EXPECT_EQ(101, secondMockModule.readFromModule(2));

	delete[] memoryPointer;
}