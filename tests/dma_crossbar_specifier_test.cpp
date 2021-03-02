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

TEST(DMACrossbarSpecifierTest, DISABLED_ResolveInputClashDetectsNoClash) {
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

TEST(DMACrossbarSpecifierTest, DISABLED_ResolveInputClashOnce) {
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

TEST(DMACrossbarSpecifierTest, DISABLED_ResolveInputClashTwice) {
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

TEST(DMACrossbarSpecifierTest, DISABLED_ResolveInputClashWithExtraSpace) {
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

TEST(DMACrossbarSpecifierTest, DISABLED_ResolveInputClashWithoutExtraSpace) {
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

TEST(DMACrossbarSpecifierTest, DISABLED_ResolveOutputClashDetectsNoClash) {
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

TEST(DMACrossbarSpecifierTest, DISABLED_ResolveOutputClashOnce) {
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

TEST(DMACrossbarSpecifierTest, DISABLED_ResolveOutputClashTwice) {
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

TEST(DMACrossbarSpecifierTest, DISABLED_ResolveOutputClashWithExtraSpace) {
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

TEST(DMACrossbarSpecifierTest, DISABLED_ResolveOutputClashWithoutExtraSpace) {
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

TEST(DMACrossbarSpecifierTest, ExtendSpecificationAsMuchAsPossible) {
  const int records_per_ddr_burst = 16;
  const std::vector<int> record_specification = {
      0, 3, -1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1, -1, 0, 17, 18};
  const int record_size = 19;

  auto extended_specification =
      DMACrossbarSpecifier::ExtendSpecificationSingleChannel(
          record_size, record_specification, records_per_ddr_burst);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  13,  -1,
           12,  15,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  29,
           30,  15,  -1,  -1,  45,  46,  47,  16,  17,  18,  19,  20,  21,  22,
           23,  24,  26,  -1,  25,  28,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  42,  43,  28,  -1,  -1,  58,  59,  60,  61,  62,  63,
           32,  33,  34,  35,  36,  37,  39,  -1,  38,  41,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  55,  56,  41,  -1,  -1,  71,  72,
           73,  74,  75,  76,  77,  78,  79,  48,  49,  50,  52,  -1,  51,  54,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  68,  69,  54,
           -1,  -1,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
           65,  -1,  64,  67,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  81,  82,  67,  -1,  -1,  97,  98,  99,  100, 101, 102, 103, 104,
           105, 106, 107, 108, 110, -1,  109, 80,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  126, 127, 80,  -1,  -1,  142, 143, 112, 113,
           114, 115, 116, 117, 118, 119, 120, 121, 123, -1,  122, 125, -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  139, 140, 125, -1,  -1,
           155, 156, 157, 158, 159, 128, 129, 130, 131, 132, 133, 134, 136, -1,
           135, 138, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  152,
           153, 138, -1,  -1,  168, 169, 170, 171, 172, 173, 174, 175, 144, 145,
           146, 147, 149, -1,  148, 151, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  165, 166, 151, -1,  -1,  181, 182, 183, 184, 185, 186,
           187, 188, 189, 190, 191, 160, 162, -1,  161, 164, -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  178, 179, 164, -1,  -1,  194, 195,
           196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 207, -1,  206, 177,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  223, 192, 177,
           -1,  -1,  239, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218,
           220, -1,  219, 222, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  236, 237, 222, -1,  -1,  252, 253, 254, 255, 224, 225, 226, 227,
           228, 229, 230, 231, 233, -1,  232, 235, -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  249, 250, 235, -1,  -1,  265, 266, 267, 268,
           269, 270, 271, 240, 241, 242, 243, 244, 246, -1,  245, 248, -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  262, 263, 248, -1,  -1,
           278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 256, 257, 259, -1,
           258, 261, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  275,
           276, 261, -1,  -1,  291, 292, 293, 294, 295, 296, 297, 298, 299, 300,
           301, 302, 272, -1,  303, 274, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  288, 289, 274, -1,  -1}));

  int const chunks_per_record = 2;
  extended_specification =
      DMACrossbarSpecifier::ExtendSpecificationMultiChannel(
          record_size, record_specification, records_per_ddr_burst,
          chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  13,  -1,
           12,  15,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  29,
           30,  15,  -1,  -1,  45,  46,  47,  16,  17,  18,  19,  20,  21,  22,
           23,  24,  26,  -1,  25,  28,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  42,  43,  28,  -1,  -1,  58,  59,  60,  61,  62,  63,
           32,  33,  34,  35,  36,  37,  39,  -1,  38,  41,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  55,  56,  41,  -1,  -1,  71,  72,
           73,  74,  75,  76,  77,  78,  79,  48,  49,  50,  52,  -1,  51,  54,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  68,  69,  54,
           -1,  -1,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
           65,  -1,  64,  67,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  81,  82,  67,  -1,  -1,  97,  98,  99,  100, 101, 102, 103, 104,
           105, 106, 107, 108, 110, -1,  109, 80,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  126, 127, 80,  -1,  -1,  142, 143, 112, 113,
           114, 115, 116, 117, 118, 119, 120, 121, 123, -1,  122, 125, -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  139, 140, 125, -1,  -1,
           155, 156, 157, 158, 159, 128, 129, 130, 131, 132, 133, 134, 136, -1,
           135, 138, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  152,
           153, 138, -1,  -1,  168, 169, 170, 171, 172, 173, 174, 175, 144, 145,
           146, 147, 149, -1,  148, 151, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  165, 166, 151, -1,  -1,  181, 182, 183, 184, 185, 186,
           187, 188, 189, 190, 191, 160, 162, -1,  161, 164, -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  178, 179, 164, -1,  -1,  194, 195,
           196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 207, -1,  206, 177,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  223, 192, 177,
           -1,  -1,  239, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218,
           220, -1,  219, 222, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  236, 237, 222, -1,  -1,  252, 253, 254, 255, 224, 225, 226, 227,
           228, 229, 230, 231, 233, -1,  232, 235, -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  249, 250, 235, -1,  -1,  265, 266, 267, 268,
           269, 270, 271, 240, 241, 242, 243, 244, 246, -1,  245, 248, -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  262, 263, 248, -1,  -1,
           278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 256, 257, 259, -1,
           258, 261, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  275,
           276, 261, -1,  -1,  291, 292, 293, 294, 295, 296, 297, 298, 299, 300,
           301, 302, 272, -1,  303, 274, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  288, 289, 274, -1,  -1}));
}

TEST(DMACrossbarSpecifierTest, ExtendSpecificationWithUnsupportedChunkCount) {
  const int records_per_ddr_burst = 8;
  const std::vector<int> record_specification = {
      0, 3,  -1, 2,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, -1, -1,
      0, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18};
  const int record_size = 19;

  auto extended_specification =
      DMACrossbarSpecifier::ExtendSpecificationSingleChannel(
          record_size, record_specification, records_per_ddr_burst);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  13,  -1,
           12,  15,  29,  29,  29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
           30,  15,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  29,  29,  29,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  45,  46,  47,  16,  17,  18,
           19,  20,  21,  22,  23,  24,  26,  -1,  25,  28,  42,  42,  42,  42,
           42,  42,  42,  42,  42,  42,  42,  42,  43,  28,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  42,  42,  42,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  58,  59,  60,  61,  62,  63,  32,  33,  34,  35,  36,  37,
           39,  -1,  38,  41,  55,  55,  55,  55,  55,  55,  55,  55,  55,  55,
           55,  55,  56,  41,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  55,  55,  55,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  71,  72,  73,  74,
           75,  76,  77,  78,  79,  48,  49,  50,  52,  -1,  51,  54,  68,  68,
           68,  68,  68,  68,  68,  68,  68,  68,  68,  68,  69,  54,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  68,
           68,  68,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,
           94,  95,  65,  -1,  64,  67,  81,  81,  81,  81,  81,  81,  81,  81,
           81,  81,  81,  81,  82,  67,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  81,  81,  81,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  97,  98,
           99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 110, -1,  109, 80,
           126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 127, 80,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  126, 126, 126, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  142, 143, 112, 113, 114, 115, 116, 117,
           118, 119, 120, 121, 123, -1,  122, 125, 139, 139, 139, 139, 139, 139,
           139, 139, 139, 139, 139, 139, 140, 125, -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  139, 139, 139, -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           155, 156, 157, 158, 159, 128, 129, 130, 131, 132, 133, 134, 136, -1,
           135, 138, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
           153, 138, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  152, 152, 152, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1}));

  int const chunks_per_record = 3;
  extended_specification =
      DMACrossbarSpecifier::ExtendSpecificationMultiChannel(
          record_size, record_specification, records_per_ddr_burst,
          chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  13,  -1,
           12,  15,  29,  29,  29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
           30,  15,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  29,  29,  29,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  45,  46,  47,  16,  17,  18,
           19,  20,  21,  22,  23,  24,  26,  -1,  25,  28,  42,  42,  42,  42,
           42,  42,  42,  42,  42,  42,  42,  42,  43,  28,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  42,  42,  42,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  58,  59,  60,  61,  62,  63,  32,  33,  34,  35,  36,  37,
           39,  -1,  38,  41,  55,  55,  55,  55,  55,  55,  55,  55,  55,  55,
           55,  55,  56,  41,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  55,  55,  55,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  71,  72,  73,  74,
           75,  76,  77,  78,  79,  48,  49,  50,  52,  -1,  51,  54,  68,  68,
           68,  68,  68,  68,  68,  68,  68,  68,  68,  68,  69,  54,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  68,
           68,  68,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,
           94,  95,  65,  -1,  64,  67,  81,  81,  81,  81,  81,  81,  81,  81,
           81,  81,  81,  81,  82,  67,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  81,  81,  81,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  97,  98,
           99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 110, -1,  109, 80,
           126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 127, 80,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  126, 126, 126, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  142, 143, 112, 113, 114, 115, 116, 117,
           118, 119, 120, 121, 123, -1,  122, 125, 139, 139, 139, 139, 139, 139,
           139, 139, 139, 139, 139, 139, 140, 125, -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  139, 139, 139, -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           155, 156, 157, 158, 159, 128, 129, 130, 131, 132, 133, 134, 136, -1,
           135, 138, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
           153, 138, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  152, 152, 152, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1}));
}

TEST(DMACrossbarSpecifierTest,
     ExtendSpecificationPlacesRecordsBasedOnStreamType) {
  const int records_per_ddr_burst = 4;
  const std::vector<int> record_specification = {
      0, 3, -1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1, -1, 0, 17, 18};
  const int record_size = 19;

  auto extended_specification =
      DMACrossbarSpecifier::ExtendSpecificationSingleChannel(
          record_size, record_specification, records_per_ddr_burst);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 13, -1, 12, 15, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 29, 30, 15, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, 45, 46, 47, 16, 17, 18, 19, 20,
           21, 22, 23, 24, 26, -1, 25, 28, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, 42, 43, 28, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, 58, 59, 60, 61, 62, 63, 32, 33, 34, 35, 36, 37, 39, -1, 38, 41,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 55, 56, 41, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 71, 72, 73, 74, 75, 76, 77,
           78, 79, 48, 49, 50, 52, -1, 51, 54, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, 68, 69, 54, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1}));

  int const chunks_per_record = 4;
  extended_specification =
      DMACrossbarSpecifier::ExtendSpecificationMultiChannel(
          record_size, record_specification, records_per_ddr_burst,
          chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 13, -1, 12, 15,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 29, 30, 15, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           45, 46, 47, 16, 17, 18, 19, 20, 21, 22, 23, 24, 26, -1, 25, 28,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 42, 43, 28, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           58, 59, 60, 61, 62, 63, 32, 33, 34, 35, 36, 37, 39, -1, 38, 41,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 55, 56, 41, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           71, 72, 73, 74, 75, 76, 77, 78, 79, 48, 49, 50, 52, -1, 51, 54,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 68, 69, 54, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}));
}

TEST(DMACrossbarSpecifierTest, ExtendSpecificationConsidersRecordSize) {
  const int records_per_ddr_burst = 16;
  const std::vector<int> record_specification = {0, 3};
  const int record_size = 19;

  auto extended_specification =
      DMACrossbarSpecifier::ExtendSpecificationSingleChannel(
          record_size, record_specification, records_per_ddr_burst);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12,  15,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 25,  28,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 38,  41,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 51,  54,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 64,  67,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 109, 80,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 122, 125,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 135, 138,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 148, 151,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 161, 164,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 206, 177,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 219, 222,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 232, 235,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 245, 248,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 258, 261,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 303, 274,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1}));

  int const chunks_per_record = 2;
  extended_specification =
      DMACrossbarSpecifier::ExtendSpecificationMultiChannel(
          record_size, record_specification, records_per_ddr_burst,
          chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12,  15,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 25,  28,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 38,  41,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 51,  54,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 64,  67,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 109, 80,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 122, 125,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 135, 138,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 148, 151,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 161, 164,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 206, 177,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 219, 222,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 232, 235,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 245, 248,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 258, 261,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 303, 274,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1}));
}

TEST(DMACrossbarSpecifierTest, ExtendOutputSpecificationAsMuchAsPossible) {
  const int records_per_ddr_burst = 16;
  const std::vector<int> record_specification = {
      0, 3, 16, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1, -1, 0, 17, 18};
  const int chunks_per_record = 2;

  auto extended_specification = DMACrossbarSpecifier::ExtendOutputSpecification(
      record_specification, records_per_ddr_burst, chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   3,   16,  2,   4,   5,   6,   7,   8,   9,   10,  11,  12,
           13,  14,  15,  -1,  -1,  0,   17,  18,  32,  35,  48,  34,  36,
           37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  -1,  -1,
           32,  49,  50,  64,  67,  80,  66,  68,  69,  70,  71,  72,  73,
           74,  75,  76,  77,  78,  79,  -1,  -1,  64,  81,  82,  96,  99,
           112, 98,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
           111, -1,  -1,  96,  113, 114, 128, 131, 144, 130, 132, 133, 134,
           135, 136, 137, 138, 139, 140, 141, 142, 143, -1,  -1,  128, 145,
           146, 160, 163, 176, 162, 164, 165, 166, 167, 168, 169, 170, 171,
           172, 173, 174, 175, -1,  -1,  160, 177, 178, 192, 195, 208, 194,
           196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, -1,
           -1,  192, 209, 210, 224, 227, 240, 226, 228, 229, 230, 231, 232,
           233, 234, 235, 236, 237, 238, 239, -1,  -1,  224, 241, 242, 256,
           259, 272, 258, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269,
           270, 271, -1,  -1,  256, 273, 274, 288, 291, 304, 290, 292, 293,
           294, 295, 296, 297, 298, 299, 300, 301, 302, 303, -1,  -1,  288,
           305, 306, 320, 323, 336, 322, 324, 325, 326, 327, 328, 329, 330,
           331, 332, 333, 334, 335, -1,  -1,  320, 337, 338, 352, 355, 368,
           354, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367,
           -1,  -1,  352, 369, 370, 384, 387, 400, 386, 388, 389, 390, 391,
           392, 393, 394, 395, 396, 397, 398, 399, -1,  -1,  384, 401, 402,
           416, 419, 432, 418, 420, 421, 422, 423, 424, 425, 426, 427, 428,
           429, 430, 431, -1,  -1,  416, 433, 434, 448, 451, 464, 450, 452,
           453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, -1,  -1,
           448, 465, 466, 480, 483, 496, 482, 484, 485, 486, 487, 488, 489,
           490, 491, 492, 493, 494, 495, -1,  -1,  480, 497, 498}));
}

TEST(DMACrossbarSpecifierTest,
     ExtendOutputSpecificationWithUnsupportedChunkCount) {
  const int records_per_ddr_burst = 8;
  const std::vector<int> record_specification = {
      0, 3, 16, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1, -1, 0, 17, 18};
  const int chunks_per_record = 3;

  auto extended_specification = DMACrossbarSpecifier::ExtendOutputSpecification(
      record_specification, records_per_ddr_burst, chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   3,   16,  2,   4,   5,   6,   7,   8,   9,   10,  11,  12,
           13,  14,  15,  -1,  -1,  0,   17,  18,  48,  51,  64,  50,  52,
           53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  -1,  -1,
           48,  65,  66,  96,  99,  112, 98,  100, 101, 102, 103, 104, 105,
           106, 107, 108, 109, 110, 111, -1,  -1,  96,  113, 114, 144, 147,
           160, 146, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158,
           159, -1,  -1,  144, 161, 162, 192, 195, 208, 194, 196, 197, 198,
           199, 200, 201, 202, 203, 204, 205, 206, 207, -1,  -1,  192, 209,
           210, 240, 243, 256, 242, 244, 245, 246, 247, 248, 249, 250, 251,
           252, 253, 254, 255, -1,  -1,  240, 257, 258, 288, 291, 304, 290,
           292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, -1,
           -1,  288, 305, 306, 336, 339, 352, 338, 340, 341, 342, 343, 344,
           345, 346, 347, 348, 349, 350, 351, -1,  -1,  336, 353, 354}));
}

TEST(DMACrossbarSpecifierTest,
     ExtendOutputSpecificationWithLessRecordsThanPossible) {
  const int records_per_ddr_burst = 8;
  const std::vector<int> record_specification = {
      0, 3, 16, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1, -1, 0, 17, 18};
  const int chunks_per_record = 2;

  auto extended_specification = DMACrossbarSpecifier::ExtendOutputSpecification(
      record_specification, records_per_ddr_burst, chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   3,   16,  2,   4,   5,   6,   7,   8,   9,   10,  11,  12,
           13,  14,  15,  -1,  -1,  0,   17,  18,  32,  35,  48,  34,  36,
           37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  -1,  -1,
           32,  49,  50,  64,  67,  80,  66,  68,  69,  70,  71,  72,  73,
           74,  75,  76,  77,  78,  79,  -1,  -1,  64,  81,  82,  96,  99,
           112, 98,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
           111, -1,  -1,  96,  113, 114, 128, 131, 144, 130, 132, 133, 134,
           135, 136, 137, 138, 139, 140, 141, 142, 143, -1,  -1,  128, 145,
           146, 160, 163, 176, 162, 164, 165, 166, 167, 168, 169, 170, 171,
           172, 173, 174, 175, -1,  -1,  160, 177, 178, 192, 195, 208, 194,
           196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, -1,
           -1,  192, 209, 210, 224, 227, 240, 226, 228, 229, 230, 231, 232,
           233, 234, 235, 236, 237, 238, 239, -1,  -1,  224, 241, 242}));
}
}  // namespace

// Fix input clashes in the tests!