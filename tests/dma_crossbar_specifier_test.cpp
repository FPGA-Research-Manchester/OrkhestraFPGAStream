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
          {0,   3,   -1,  2,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,
           14,  15,  -1,  -1,  0,   17,  18,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  19,  22,  -1,  21,  23,  24,  25,  26,  27,  28,
           29,  30,  31,  32,  33,  34,  -1,  -1,  19,  36,  37,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  38,  41,  -1,  40,  42,  43,
           44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  -1,  -1,  38,  55,
           56,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  57,  60,
           -1,  59,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,
           -1,  -1,  57,  74,  75,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  76,  79,  -1,  78,  80,  81,  82,  83,  84,  85,  86,  87,
           88,  89,  90,  91,  -1,  -1,  76,  93,  94,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  95,  98,  -1,  97,  99,  100, 101, 102,
           103, 104, 105, 106, 107, 108, 109, 110, -1,  -1,  95,  112, 113, -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  114, 117, -1,  116,
           118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, -1,  -1,
           114, 131, 132, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           133, 136, -1,  135, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146,
           147, 148, -1,  -1,  133, 150, 151, -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  152, 155, -1,  154, 156, 157, 158, 159, 160, 161,
           162, 163, 164, 165, 166, 167, -1,  -1,  152, 169, 170, -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  171, 174, -1,  173, 175, 176,
           177, 178, 179, 180, 181, 182, 183, 184, 185, 186, -1,  -1,  171, 188,
           189, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  190, 193,
           -1,  192, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205,
           -1,  -1,  190, 207, 208, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  209, 212, -1,  211, 213, 214, 215, 216, 217, 218, 219, 220,
           221, 222, 223, 224, -1,  -1,  209, 226, 227, -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  228, 231, -1,  230, 232, 233, 234, 235,
           236, 237, 238, 239, 240, 241, 242, 243, -1,  -1,  228, 245, 246, -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  247, 250, -1,  249,
           251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, -1,  -1,
           247, 264, 265, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           266, 269, -1,  268, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279,
           280, 281, -1,  -1,  266, 283, 284, -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  285, 288, -1,  287, 289, 290, 291, 292, 293, 294,
           295, 296, 297, 298, 299, 300, -1,  -1,  285, 302, 303, -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1}));

  int const chunks_per_record = 2;
  extended_specification =
      DMACrossbarSpecifier::ExtendSpecificationMultiChannel(
          record_size, record_specification, records_per_ddr_burst,
          chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   3,   -1,  2,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,
           14,  15,  -1,  -1,  0,   17,  18,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  19,  22,  -1,  21,  23,  24,  25,  26,  27,  28,
           29,  30,  31,  32,  33,  34,  -1,  -1,  19,  36,  37,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  38,  41,  -1,  40,  42,  43,
           44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  -1,  -1,  38,  55,
           56,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  57,  60,
           -1,  59,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,
           -1,  -1,  57,  74,  75,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  76,  79,  -1,  78,  80,  81,  82,  83,  84,  85,  86,  87,
           88,  89,  90,  91,  -1,  -1,  76,  93,  94,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  95,  98,  -1,  97,  99,  100, 101, 102,
           103, 104, 105, 106, 107, 108, 109, 110, -1,  -1,  95,  112, 113, -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  114, 117, -1,  116,
           118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, -1,  -1,
           114, 131, 132, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           133, 136, -1,  135, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146,
           147, 148, -1,  -1,  133, 150, 151, -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  152, 155, -1,  154, 156, 157, 158, 159, 160, 161,
           162, 163, 164, 165, 166, 167, -1,  -1,  152, 169, 170, -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  171, 174, -1,  173, 175, 176,
           177, 178, 179, 180, 181, 182, 183, 184, 185, 186, -1,  -1,  171, 188,
           189, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  190, 193,
           -1,  192, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205,
           -1,  -1,  190, 207, 208, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  209, 212, -1,  211, 213, 214, 215, 216, 217, 218, 219, 220,
           221, 222, 223, 224, -1,  -1,  209, 226, 227, -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  228, 231, -1,  230, 232, 233, 234, 235,
           236, 237, 238, 239, 240, 241, 242, 243, -1,  -1,  228, 245, 246, -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  247, 250, -1,  249,
           251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, -1,  -1,
           247, 264, 265, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           266, 269, -1,  268, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279,
           280, 281, -1,  -1,  266, 283, 284, -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  285, 288, -1,  287, 289, 290, 291, 292, 293, 294,
           295, 296, 297, 298, 299, 300, -1,  -1,  285, 302, 303, -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1}));
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
          {0,   3,   -1,  2,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,
           14,  15,  -1,  -1,  0,   17,  18,  18,  18,  18,  18,  18,  18,  18,
           18,  18,  18,  18,  18,  18,  18,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  19,  22,  -1,  21,  23,  24,
           25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  -1,  -1,  19,  36,
           37,  37,  37,  37,  37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
           37,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  38,  41,  -1,  40,  42,  43,  44,  45,  46,  47,  48,  49,
           50,  51,  52,  53,  -1,  -1,  38,  55,  56,  56,  56,  56,  56,  56,
           56,  56,  56,  56,  56,  56,  56,  56,  56,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  57,  60,  -1,  59,
           61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  -1,  -1,
           57,  74,  75,  75,  75,  75,  75,  75,  75,  75,  75,  75,  75,  75,
           75,  75,  75,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  76,  79,  -1,  78,  80,  81,  82,  83,  84,  85,
           86,  87,  88,  89,  90,  91,  -1,  -1,  76,  93,  94,  94,  94,  94,
           94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  95,  98,
           -1,  97,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
           -1,  -1,  95,  112, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113,
           113, 113, 113, 113, 113, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  114, 117, -1,  116, 118, 119, 120, 121,
           122, 123, 124, 125, 126, 127, 128, 129, -1,  -1,  114, 131, 132, 132,
           132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           133, 136, -1,  135, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146,
           147, 148, -1,  -1,  133, 150, 151, 151, 151, 151, 151, 151, 151, 151,
           151, 151, 151, 151, 151, 151, 151, -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1}));

  int const chunks_per_record = 3;
  extended_specification =
      DMACrossbarSpecifier::ExtendSpecificationMultiChannel(
          record_size, record_specification, records_per_ddr_burst,
          chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   3,   -1,  2,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,
           14,  15,  -1,  -1,  0,   17,  18,  18,  18,  18,  18,  18,  18,  18,
           18,  18,  18,  18,  18,  18,  18,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  19,  22,  -1,  21,  23,  24,
           25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  -1,  -1,  19,  36,
           37,  37,  37,  37,  37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
           37,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  38,  41,  -1,  40,  42,  43,  44,  45,  46,  47,  48,  49,
           50,  51,  52,  53,  -1,  -1,  38,  55,  56,  56,  56,  56,  56,  56,
           56,  56,  56,  56,  56,  56,  56,  56,  56,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  57,  60,  -1,  59,
           61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  -1,  -1,
           57,  74,  75,  75,  75,  75,  75,  75,  75,  75,  75,  75,  75,  75,
           75,  75,  75,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  76,  79,  -1,  78,  80,  81,  82,  83,  84,  85,
           86,  87,  88,  89,  90,  91,  -1,  -1,  76,  93,  94,  94,  94,  94,
           94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  95,  98,
           -1,  97,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
           -1,  -1,  95,  112, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113,
           113, 113, 113, 113, 113, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  114, 117, -1,  116, 118, 119, 120, 121,
           122, 123, 124, 125, 126, 127, 128, 129, -1,  -1,  114, 131, 132, 132,
           132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
           133, 136, -1,  135, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146,
           147, 148, -1,  -1,  133, 150, 151, 151, 151, 151, 151, 151, 151, 151,
           151, 151, 151, 151, 151, 151, 151, -1,  -1,  -1,  -1,  -1,  -1,  -1,
           -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
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
          {0,  3,  -1, 2,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, -1,
           -1, 0,  17, 18, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, 19, 22, -1, 21, 23, 24, 25, 26,
           27, 28, 29, 30, 31, 32, 33, 34, -1, -1, 19, 36, 37, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, 38, 41, -1, 40, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
           -1, -1, 38, 55, 56, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 57, 60, -1, 59, 61, 62, 63,
           64, 65, 66, 67, 68, 69, 70, 71, 72, -1, -1, 57, 74, 75, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
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
          {0,  3,  -1, 2,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
           -1, -1, 0,  17, 18, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           19, 22, -1, 21, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
           -1, -1, 19, 36, 37, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           38, 41, -1, 40, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
           -1, -1, 38, 55, 56, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           57, 60, -1, 59, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
           -1, -1, 57, 74, 75, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
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
          {0,   3,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           19,  22,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           38,  41,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           57,  60,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           76,  79,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           95,  98,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           114, 117, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           133, 136, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           152, 155, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           171, 174, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           190, 193, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           209, 212, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           228, 231, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           247, 250, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           266, 269, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           285, 288, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}));

  int const chunks_per_record = 2;
  extended_specification =
      DMACrossbarSpecifier::ExtendSpecificationMultiChannel(
          record_size, record_specification, records_per_ddr_burst,
          chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   3,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           19,  22,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           38,  41,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           57,  60,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           76,  79,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           95,  98,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           114, 117, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           133, 136, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           152, 155, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           171, 174, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           190, 193, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           209, 212, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           228, 231, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           247, 250, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           266, 269, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           285, 288, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}));
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