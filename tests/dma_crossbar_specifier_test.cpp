#include "dma_crossbar_specifier.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
namespace {

TEST(DMACrossbarSpecifierTest, SpecifierDetectsInputClash) {
  // Normal
  EXPECT_FALSE(DMACrossbarSpecifier::IsInputClashing({0, 1, 2}));
  // Legal diagonal move
  EXPECT_FALSE(DMACrossbarSpecifier::IsInputClashing({2, 1, 16}));
  // Duplication
  EXPECT_FALSE(DMACrossbarSpecifier::IsInputClashing({1, 1, 2, 2, 2}));
  // Garbage data
  EXPECT_FALSE(DMACrossbarSpecifier::IsInputClashing({-1, -1, 0, 1, 2}));
  // Simple clash
  EXPECT_TRUE(DMACrossbarSpecifier::IsInputClashing({1, 17}));
  // 2 Chunk clash
  EXPECT_TRUE(DMACrossbarSpecifier::IsInputClashing(
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 15, 0, 17}));
}

TEST(DMACrossbarSpecifierTest, SpecifierDetectsOutputClash) {
  // Normal
  EXPECT_FALSE(DMACrossbarSpecifier::IsOutputClashing(
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17}));
  // Legal diagonal move with garbage data
  EXPECT_FALSE(DMACrossbarSpecifier::IsOutputClashing(
      {17, -1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1, 0}));
  // Duplication
  EXPECT_FALSE(DMACrossbarSpecifier::IsOutputClashing(
      {0, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}));
  // 2 Chunk clash
  EXPECT_TRUE(DMACrossbarSpecifier::IsOutputClashing(
      {1, 3, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0, 17, 18}));
  // Duplication clash
  EXPECT_TRUE(DMACrossbarSpecifier::IsOutputClashing(
      {0, 3, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0, 17, 18}));
}

}  // namespace
