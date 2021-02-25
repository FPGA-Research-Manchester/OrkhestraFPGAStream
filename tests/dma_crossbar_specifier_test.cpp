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

TEST(DMACrossbarSpecifierTest, SpecifierDetectsOutputOverwrites) {
  // Normal
  EXPECT_FALSE(DMACrossbarSpecifier::IsOutputOverwritingData(
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17}));
  // Legal overwrites
  EXPECT_FALSE(DMACrossbarSpecifier::IsOutputOverwritingData(
      {0, 0, 2, 3, 0, 5, 15, 7, 8, 9, 2, 11, 1, 13, 14, 15, 16, 16}));
  // Garbage data
  EXPECT_FALSE(DMACrossbarSpecifier::IsOutputOverwritingData(
      {0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
       1}));
  // Too much duplication
  EXPECT_TRUE(DMACrossbarSpecifier::IsOutputOverwritingData(
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 8}));
  EXPECT_TRUE(DMACrossbarSpecifier::IsOutputOverwritingData(
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18}));
}

TEST(DMACrossbarSpecifierTest, ResolveInputClashDetectsNoClash) {
  // Setup input
  int records_per_ddr_burst = 32;
  std::vector<int> record_specification = {0, 1, 2};
  const int record_size = 3;

  // Run
  DMACrossbarSpecifier::ResolveInputClashesSingleChannel(
      record_size, record_specification, records_per_ddr_burst);

  // Assert everything is the same
  ASSERT_EQ(32, records_per_ddr_burst);
  ASSERT_THAT(record_specification, testing::ElementsAreArray({0, 1, 2}));

  // Setup additional input for multi-channel streams.
  int chunks_per_record = 1;

  DMACrossbarSpecifier::ResolveInputClashesMultiChannel(
      record_size, record_specification, records_per_ddr_burst,
      chunks_per_record);

  // Assert everything is still the same
  ASSERT_EQ(1, chunks_per_record);
  ASSERT_THAT(record_specification, testing::ElementsAreArray({0, 1, 2}));
}

TEST(DMACrossbarSpecifierTest, ResolveInputClashOnce) {
  // Resolve clash at position 15 with 16 being the same modulo as 0
  int records_per_ddr_burst = 16;
  std::vector<int> record_specification = {0,  1,  2,  3,  4,  5,  6,  7, 8, 9,
                                           10, 11, 12, 13, 14, 16, 15, 5, 17};
  const int record_size = 19;

  DMACrossbarSpecifier::ResolveInputClashesSingleChannel(
      record_size, record_specification, records_per_ddr_burst);

  ASSERT_EQ(16, records_per_ddr_burst);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  1,  2,  3,  4,  5,  6,  7, 8,  9,
                                 10, 11, 12, 13, 14, -1, 15, 5, 17, 16}));

  int chunks_per_record = 2;
  DMACrossbarSpecifier::ResolveInputClashesMultiChannel(
      record_size, record_specification, records_per_ddr_burst,
      chunks_per_record);

  ASSERT_EQ(2, chunks_per_record);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  1,  2,  3,  4,  5,  6,  7, 8,  9,
                                 10, 11, 12, 13, 14, -1, 15, 5, 17, 16}));
}

TEST(DMACrossbarSpecifierTest, ResolveInputClashTwice) {
  // Resolve clashes at position 14 and 15.
  int records_per_ddr_burst = 16;
  std::vector<int> record_specification = {0,  1,  2,  3,  4,  5,  6,  7, 8, 9,
                                           10, 11, 12, 13, 17, 16, 15, 5, 17};
  const int record_size = 19;

  DMACrossbarSpecifier::ResolveInputClashesSingleChannel(
      record_size, record_specification, records_per_ddr_burst);

  ASSERT_EQ(16, records_per_ddr_burst);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  1,  2,  3,  4,  5,  6, 7,  8,  9, 10,
                                 11, 12, 13, -1, -1, 15, 5, 17, 16, 17}));

  int chunks_per_record = 2;
  DMACrossbarSpecifier::ResolveInputClashesMultiChannel(
      record_size, record_specification, records_per_ddr_burst,
      chunks_per_record);

  ASSERT_EQ(2, chunks_per_record);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  1,  2,  3,  4,  5,  6, 7,  8,  9, 10,
                                 11, 12, 13, -1, -1, 15, 5, 17, 16, 17}));
}

TEST(DMACrossbarSpecifierTest, ResolveInputClashWithExtraSpace) {
  // Resolve clash at pos 15 first but then 0 and 16 are clashing in the second
  // chunk. 16 gets pushed to the extra chunk.
  int records_per_ddr_burst = 16;
  std::vector<int> record_specification = {0,  1,  2,  3,  4,  5,  6,  7, 8, 9,
                                           10, 11, 12, 13, 14, 16, 15, 0, 17};
  const int record_size = 19;

  DMACrossbarSpecifier::ResolveInputClashesSingleChannel(
      record_size, record_specification, records_per_ddr_burst);

  // Records per DDR burst gets halved
  ASSERT_EQ(8, records_per_ddr_burst);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                 11, 12, 13, 14, -1, 15, 0,  17, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16}));

  int chunks_per_record = 2;
  records_per_ddr_burst = 16;
  DMACrossbarSpecifier::ResolveInputClashesMultiChannel(
      record_size, record_specification, records_per_ddr_burst,
      chunks_per_record);

  // Chunks per record gets increased.
  ASSERT_EQ(3, chunks_per_record);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                 11, 12, 13, 14, -1, 15, 0,  17, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16}));
}

TEST(DMACrossbarSpecifierTest, ResolveInputClashWithoutExtraSpace) {
  // Resolve clash at pos 15 first but then 0 and 16 are clashing in the second
  // chunk. 16 gets pushed to the deleted chunk.
  int records_per_ddr_burst = 8;
  std::vector<int> record_specification = {0,  1,  2,  3,  4,  5,  6,  7, 8, 9,
                                           10, 11, 12, 13, 14, 16, 15, 0, 17};
  const int record_size = 33;

  DMACrossbarSpecifier::ResolveInputClashesSingleChannel(
      record_size, record_specification, records_per_ddr_burst);

  ASSERT_EQ(8, records_per_ddr_burst);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                 11, 12, 13, 14, -1, 15, 0,  17, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16}));

  int chunks_per_record = 3;
  records_per_ddr_burst = 8;
  DMACrossbarSpecifier::ResolveInputClashesMultiChannel(
      record_size, record_specification, records_per_ddr_burst,
      chunks_per_record);

  ASSERT_EQ(3, chunks_per_record);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                 11, 12, 13, 14, -1, 15, 0,  17, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16}));
}

TEST(DMACrossbarSpecifierTest, ResolveOutputClashDetectsNoClash) {
  // No clash - input should stay as it is.
  int records_per_ddr_burst = 16;
  std::vector<int> record_specification = {0, 1,  2,  3,  4,  5,  6,  7,  8,
                                           9, 10, 11, 12, 13, 14, 15, 16, 17};
  const int record_size = 18;

  DMACrossbarSpecifier::ResolveOutputClashesSingleChannel(
      record_size, record_specification, records_per_ddr_burst);

  ASSERT_EQ(16, records_per_ddr_burst);
  ASSERT_THAT(record_specification,
              testing::ElementsAreArray({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                                         12, 13, 14, 15, 16, 17}));
}

TEST(DMACrossbarSpecifierTest, ResolveOutputClashThrowsError) {
  // Unresolvable clash because of too many duplications with chunk 0 data.
  int records_per_ddr_burst = 16;
  std::vector<int> record_specification = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 18};
  const int record_size = 19;

  ASSERT_THROW(DMACrossbarSpecifier::ResolveOutputClashesSingleChannel(
                   record_size, record_specification, records_per_ddr_burst),
               std::runtime_error);
}

TEST(DMACrossbarSpecifierTest, ResolveOutputClashOnce) {
  // Clash at pos 0 with 0 and 0
  int records_per_ddr_burst = 16;
  std::vector<int> record_specification = {0,  16, 3,  2,  4,  5,  6, 7,  8, 9,
                                           10, 11, 12, 13, 14, 15, 0, 17, 18};
  const int record_size = 19;

  DMACrossbarSpecifier::ResolveOutputClashesSingleChannel(
      record_size, record_specification, records_per_ddr_burst);

  ASSERT_EQ(16, records_per_ddr_burst);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  16, 3,  2,  4,  5,  6,  7, 8,  9,
                                 10, 11, 12, 13, 14, 15, -1, 0, 17, 18}));
}

TEST(DMACrossbarSpecifierTest, ResolveOutputClashTwice) {
  // Clash at pos 0 with 0 and 0 and then at pos 1 with 0 and 3
  int records_per_ddr_burst = 16;
  std::vector<int> record_specification = {0,  3,  16, 2,  4,  5,  6, 7,  8, 9,
                                           10, 11, 12, 13, 14, 15, 0, 17, 18};
  const int record_size = 19;

  DMACrossbarSpecifier::ResolveOutputClashesSingleChannel(
      record_size, record_specification, records_per_ddr_burst);

  ASSERT_EQ(16, records_per_ddr_burst);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  3,  16, 2,  4,  5,  6,  7, 8,  9, 10,
                                 11, 12, 13, 14, 15, -1, -1, 0, 17, 18}));
}

TEST(DMACrossbarSpecifierTest, ResolveOutputClashWithExtraSpace) {
  // Clash at pos 3 with 2 and 0. The 0 will have to get shifted to a new chunk
  int records_per_ddr_burst = 16;
  std::vector<int> record_specification = {
      0, 3, 16, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 18, 19, 0};
  const int record_size = 20;

  DMACrossbarSpecifier::ResolveOutputClashesSingleChannel(
      record_size, record_specification, records_per_ddr_burst);

  ASSERT_EQ(8, records_per_ddr_burst);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  3,  16, 2,  4,  5,  6,  7,  8,  9,  10, 11,
                                 12, 13, 14, 15, 17, 18, -1, 19, -1, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0}));
}

TEST(DMACrossbarSpecifierTest, ResolveOutputClashWithoutExtraSpace) {
  // Clash at pos 3 with 2 and 0. The 0 will have to get shifted to the deleted
  // chunk
  int records_per_ddr_burst = 8;
  std::vector<int> record_specification = {
      0, 3, 16, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 18, 19, 0};
  const int record_size = 33;

  DMACrossbarSpecifier::ResolveOutputClashesSingleChannel(
      record_size, record_specification, records_per_ddr_burst);

  ASSERT_EQ(8, records_per_ddr_burst);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  3,  16, 2,  4,  5,  6,  7,  8,  9,  10, 11,
                                 12, 13, 14, 15, 17, 18, -1, 19, -1, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0}));
}

}  // namespace
