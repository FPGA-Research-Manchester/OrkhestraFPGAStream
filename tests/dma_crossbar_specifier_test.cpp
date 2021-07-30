/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "dma_crossbar_specifier.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
namespace {

TEST(DMACrossbarSpecifierTest, SpecifierDetectsInputClash) {
  // Normal
  EXPECT_FALSE(dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsInputClashing(
      {0, 1, 2}));
  // Legal diagonal move
  EXPECT_FALSE(dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsInputClashing(
      {2, 1, 16}));
  // Duplication
  EXPECT_FALSE(dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsInputClashing(
      {1, 1, 2, 2, 2}));
  // Garbage data
  EXPECT_FALSE(dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsInputClashing(
      {-1, -1, 0, 1, 2}));
  // Simple clash
  EXPECT_TRUE(dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsInputClashing(
      {1, 17}));
  // 2 Chunk clash
  EXPECT_TRUE(dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsInputClashing(
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 15, 0, 17}));
}

TEST(DMACrossbarSpecifierTest, SpecifierDetectsOutputClash) {
  // Normal
  EXPECT_FALSE(
      dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsOutputClashing(
          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17}));
  // Legal diagonal move with garbage data
  EXPECT_FALSE(
      dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsOutputClashing(
          {17, -1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1, 0}));
  // Duplication
  EXPECT_FALSE(
      dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsOutputClashing(
          {0, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}));
  // 2 Chunk clash
  EXPECT_TRUE(dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsOutputClashing(
      {1, 3, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0, 17, 18}));
  // Duplication clash
  EXPECT_TRUE(dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsOutputClashing(
      {0, 3, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0, 17, 18}));
}

TEST(DMACrossbarSpecifierTest, SpecifierDetectsOutputOverwrites) {
  // Normal
  EXPECT_FALSE(
      dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsOutputOverwritingData(
          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17}));
  // Legal overwrites
  EXPECT_FALSE(
      dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsOutputOverwritingData(
          {0, 0, 2, 3, 0, 5, 15, 7, 8, 9, 2, 11, 1, 13, 14, 15, 16, 16}));
  // Garbage data
  EXPECT_FALSE(
      dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsOutputOverwritingData(
          {0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, 1}));
  // Too much duplication
  EXPECT_TRUE(
      dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsOutputOverwritingData(
          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 8}));
  EXPECT_TRUE(
      dbmstodspi::fpga_managing::DMACrossbarSpecifier::IsOutputOverwritingData(
          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18}));
}

TEST(DMACrossbarSpecifierTest, DISABLED_ResolveInputClashDetectsNoClash) {
  // Setup input
  int records_per_ddr_burst = 32;
  std::vector<int> record_specification = {0, 1, 2};
  const int record_size = 3;

  // Run
  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveInputClashesSingleChannel(record_size, record_specification,
                                       records_per_ddr_burst);

  // Assert everything is the same
  ASSERT_EQ(32, records_per_ddr_burst);
  ASSERT_THAT(record_specification, testing::ElementsAreArray({0, 1, 2}));

  // Setup additional input for multi-channel streams.
  int chunks_per_record = 1;

  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveInputClashesMultiChannel(record_size, record_specification,
                                      records_per_ddr_burst, chunks_per_record);

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

  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveInputClashesSingleChannel(record_size, record_specification,
                                       records_per_ddr_burst);

  ASSERT_EQ(16, records_per_ddr_burst);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  1,  2,  3,  4,  5,  6,  7, 8,  9,
                                 10, 11, 12, 13, 14, -1, 15, 5, 17, 16}));

  int chunks_per_record = 2;
  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveInputClashesMultiChannel(record_size, record_specification,
                                      records_per_ddr_burst, chunks_per_record);

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

  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveInputClashesSingleChannel(record_size, record_specification,
                                       records_per_ddr_burst);

  ASSERT_EQ(16, records_per_ddr_burst);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  1,  2,  3,  4,  5,  6, 7,  8,  9, 10,
                                 11, 12, 13, -1, -1, 15, 5, 17, 16, 17}));

  int chunks_per_record = 2;
  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveInputClashesMultiChannel(record_size, record_specification,
                                      records_per_ddr_burst, chunks_per_record);

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

  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveInputClashesSingleChannel(record_size, record_specification,
                                       records_per_ddr_burst);

  // Records per DDR burst gets halved
  ASSERT_EQ(8, records_per_ddr_burst);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                 11, 12, 13, 14, -1, 15, 0,  17, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16}));

  int chunks_per_record = 2;
  records_per_ddr_burst = 16;
  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveInputClashesMultiChannel(record_size, record_specification,
                                      records_per_ddr_burst, chunks_per_record);

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

  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveInputClashesSingleChannel(record_size, record_specification,
                                       records_per_ddr_burst);

  ASSERT_EQ(8, records_per_ddr_burst);
  ASSERT_THAT(
      record_specification,
      testing::ElementsAreArray({0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                 11, 12, 13, 14, -1, 15, 0,  17, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16}));

  int chunks_per_record = 3;
  records_per_ddr_burst = 8;
  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveInputClashesMultiChannel(record_size, record_specification,
                                      records_per_ddr_burst, chunks_per_record);

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

  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveOutputClashesSingleChannel(record_size, record_specification,
                                        records_per_ddr_burst);

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

  ASSERT_THROW(
      dbmstodspi::fpga_managing::DMACrossbarSpecifier::
          ResolveOutputClashesSingleChannel(record_size, record_specification,
                                            records_per_ddr_burst),
      std::runtime_error);
}

TEST(DMACrossbarSpecifierTest, DISABLED_ResolveOutputClashOnce) {
  // Clash at pos 0 with 0 and 0
  int records_per_ddr_burst = 16;
  std::vector<int> record_specification = {0,  16, 3,  2,  4,  5,  6, 7,  8, 9,
                                           10, 11, 12, 13, 14, 15, 0, 17, 18};
  const int record_size = 19;

  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveOutputClashesSingleChannel(record_size, record_specification,
                                        records_per_ddr_burst);

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

  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveOutputClashesSingleChannel(record_size, record_specification,
                                        records_per_ddr_burst);

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

  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveOutputClashesSingleChannel(record_size, record_specification,
                                        records_per_ddr_burst);

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

  dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ResolveOutputClashesSingleChannel(record_size, record_specification,
                                        records_per_ddr_burst);

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

  auto extended_specification = dbmstodspi::fpga_managing::
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
  extended_specification = dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ExtendSpecificationMultiChannel(record_size, record_specification,
                                      records_per_ddr_burst, chunks_per_record);

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

  auto extended_specification = dbmstodspi::fpga_managing::
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
  extended_specification = dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ExtendSpecificationMultiChannel(record_size, record_specification,
                                      records_per_ddr_burst, chunks_per_record);

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

  auto extended_specification = dbmstodspi::fpga_managing::
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
  extended_specification = dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ExtendSpecificationMultiChannel(record_size, record_specification,
                                      records_per_ddr_burst, chunks_per_record);

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

  auto extended_specification = dbmstodspi::fpga_managing::
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
  extended_specification = dbmstodspi::fpga_managing::DMACrossbarSpecifier::
      ExtendSpecificationMultiChannel(record_size, record_specification,
                                      records_per_ddr_burst, chunks_per_record);

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

  auto extended_specification = dbmstodspi::fpga_managing::
      DMACrossbarSpecifier::ExtendOutputSpecification(
          record_specification, records_per_ddr_burst, chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  13,  31,
           12,  15,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  29,
           30,  15,  -1,  -1,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,
           42,  43,  45,  63,  44,  47,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  61,  62,  47,  -1,  -1,  64,  65,  66,  67,  68,  69,
           70,  71,  72,  73,  74,  75,  77,  95,  76,  79,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  93,  94,  79,  -1,  -1,  96,  97,
           98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 109, 127, 108, 111,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  125, 126, 111,
           -1,  -1,  128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
           141, 159, 140, 143, -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  157, 158, 143, -1,  -1,  160, 161, 162, 163, 164, 165, 166, 167,
           168, 169, 170, 171, 173, 191, 172, 175, -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  189, 190, 175, -1,  -1,  192, 193, 194, 195,
           196, 197, 198, 199, 200, 201, 202, 203, 205, 223, 204, 207, -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  221, 222, 207, -1,  -1,
           224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 237, 255,
           236, 239, -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  253,
           254, 239, -1,  -1,  256, 257, 258, 259, 260, 261, 262, 263, 264, 265,
           266, 267, 269, 287, 268, 271, -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  285, 286, 271, -1,  -1,  288, 289, 290, 291, 292, 293,
           294, 295, 296, 297, 298, 299, 301, 319, 300, 303, -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  317, 318, 303, -1,  -1,  320, 321,
           322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 333, 351, 332, 335,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  349, 350, 335,
           -1,  -1,  352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363,
           365, 383, 364, 367, -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  381, 382, 367, -1,  -1,  384, 385, 386, 387, 388, 389, 390, 391,
           392, 393, 394, 395, 397, 415, 396, 399, -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  413, 414, 399, -1,  -1,  416, 417, 418, 419,
           420, 421, 422, 423, 424, 425, 426, 427, 429, 447, 428, 431, -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  445, 446, 431, -1,  -1,
           448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 461, 479,
           460, 463, -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  477,
           478, 463, -1,  -1,  480, 481, 482, 483, 484, 485, 486, 487, 488, 489,
           490, 491, 493, 511, 492, 495, -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  509, 510, 495, -1,  -1}));
}

TEST(DMACrossbarSpecifierTest,
     ExtendOutputSpecificationWithUnsupportedChunkCount) {
  const int records_per_ddr_burst = 8;
  const std::vector<int> record_specification = {
      0, 3, 16, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1, -1, 0, 17, 18};
  const int chunks_per_record = 3;

  auto extended_specification = dbmstodspi::fpga_managing::
      DMACrossbarSpecifier::ExtendOutputSpecification(
          record_specification, records_per_ddr_burst, chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  13,  31,
           12,  15,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  29,
           30,  15,  -1,  -1,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  48,  49,  50,  51,  52,  53,
           54,  55,  56,  57,  58,  59,  61,  79,  60,  63,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  77,  78,  63,  -1,  -1,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107,
           109, 127, 108, 111, -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  125, 126, 111, -1,  -1,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  144, 145, 146, 147,
           148, 149, 150, 151, 152, 153, 154, 155, 157, 175, 156, 159, -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  173, 174, 159, -1,  -1,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  192, 193, 194, 195, 196, 197, 198, 199, 200, 201,
           202, 203, 205, 223, 204, 207, -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  221, 222, 207, -1,  -1,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  240, 241,
           242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 253, 271, 252, 255,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  269, 270, 255,
           -1,  -1,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  288, 289, 290, 291, 292, 293, 294, 295,
           296, 297, 298, 299, 301, 319, 300, 303, -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  317, 318, 303, -1,  -1,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 349, 367,
           348, 351, -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  365,
           366, 351, -1,  -1,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2}));
}

TEST(DMACrossbarSpecifierTest,
     ExtendOutputSpecificationWithLessRecordsThanPossible) {
  const int records_per_ddr_burst = 8;
  const std::vector<int> record_specification = {
      0, 3, 16, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1, -1, 0, 17, 18};
  const int chunks_per_record = 2;

  auto extended_specification = dbmstodspi::fpga_managing::
      DMACrossbarSpecifier::ExtendOutputSpecification(
          record_specification, records_per_ddr_burst, chunks_per_record);

  ASSERT_THAT(
      extended_specification,
      testing::ElementsAreArray(
          {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  13,  31,
           12,  15,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  29,
           30,  15,  -1,  -1,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  32,  33,  34,  35,  36,  37,
           38,  39,  40,  41,  42,  43,  45,  63,  44,  47,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  61,  62,  47,  -1,  -1,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,
           77,  95,  76,  79,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  93,  94,  79,  -1,  -1,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  96,  97,  98,  99,
           100, 101, 102, 103, 104, 105, 106, 107, 109, 127, 108, 111, -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  125, 126, 111, -1,  -1,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  128, 129, 130, 131, 132, 133, 134, 135, 136, 137,
           138, 139, 141, 159, 140, 143, -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  157, 158, 143, -1,  -1,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  160, 161,
           162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 173, 191, 172, 175,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  189, 190, 175,
           -1,  -1,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  192, 193, 194, 195, 196, 197, 198, 199,
           200, 201, 202, 203, 205, 223, 204, 207, -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  221, 222, 207, -1,  -1,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 237, 255,
           236, 239, -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  253,
           254, 239, -1,  -1,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
           -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2}));
}

TEST(DMACrossbarSpecifierTest,
     SpecificationExtensionThrowsErrorWithTooManyRecords) {
  const int records_per_ddr_burst = 8;
  const std::vector<int> record_specification = {
      0, 3, 16, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1, -1, 0, 17, 18};
  const int chunks_per_record = 8;

  ASSERT_THROW(
      dbmstodspi::fpga_managing::DMACrossbarSpecifier::
          ExtendOutputSpecification(record_specification, records_per_ddr_burst,
                                    chunks_per_record),
      std::runtime_error);
}

}  // namespace

// Fix input clashes in the tests!