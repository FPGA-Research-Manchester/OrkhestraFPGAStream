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

#include "types_converter.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
namespace {

using orkhestrafs::dbmstodspi::TypesConverter;
TEST(TypesConverterTest, IntToInt) {
  std::vector<uint32_t> converted_data_vector;
  TypesConverter::ConvertIntegerValuesToIntegerData("123",
                                                    converted_data_vector, 0);
  ASSERT_EQ(1, converted_data_vector.size());
  ASSERT_EQ(123, converted_data_vector.at(0));
}
TEST(TypesConverterTest, VarcharToInt) {
  std::vector<uint32_t> converted_data_vector;
  TypesConverter::ConvertStringValuesToIntegerData("123", converted_data_vector,
                                                   1);
  ASSERT_EQ(1, converted_data_vector.size());
  ASSERT_EQ(825373440, converted_data_vector.at(0));
}
TEST(TypesConverterTest, NullToInt) {
  std::vector<uint32_t> converted_data_vector;
  TypesConverter::ConvertNullValuesToIntegerData("", converted_data_vector, 1);
  ASSERT_EQ(1, converted_data_vector.size());
  ASSERT_EQ(0, converted_data_vector.at(0));
}
TEST(TypesConverterTest, DecimalToInt) {
  std::vector<uint32_t> converted_data_vector;
  TypesConverter::ConvertDecimalValuesToIntegerData("123.123",
                                                    converted_data_vector, 0);
  ASSERT_EQ(2, converted_data_vector.size());
  ASSERT_EQ(0, converted_data_vector.at(0));
  ASSERT_EQ(12312, converted_data_vector.at(1));
}
TEST(TypesConverterTest, DateToInt) {
  std::vector<uint32_t> converted_data_vector;
  TypesConverter::ConvertDateValuesToIntegerData("12-30-11",
                                                 converted_data_vector, 0);
  ASSERT_EQ(1, converted_data_vector.size());
  ASSERT_EQ(123011, converted_data_vector.at(0));
}

TEST(TypesConverterTest, IntToString) {
  std::vector<std::string> converted_data_vector;
  TypesConverter::ConvertIntegerValuesToString({123}, converted_data_vector);
  ASSERT_EQ(1, converted_data_vector.size());
  ASSERT_EQ("123", converted_data_vector.at(0));
}
TEST(TypesConverterTest, VarcharToString) {
  std::vector<std::string> converted_data_vector;
  TypesConverter::ConvertStringValuesToString({825373440},
                                              converted_data_vector);
  ASSERT_EQ(1, converted_data_vector.size());
  ASSERT_EQ("123", converted_data_vector.at(0));
}
TEST(TypesConverterTest, NullToString) {
  std::vector<std::string> converted_data_vector;
  TypesConverter::ConvertNullValuesToString({0}, converted_data_vector);
  ASSERT_EQ(1, converted_data_vector.size());
  ASSERT_EQ("", converted_data_vector.at(0));
}
TEST(TypesConverterTest, DecimalToString) {
  std::vector<std::string> converted_data_vector;
  TypesConverter::ConvertDecimalValuesToString({0, 12312},
                                               converted_data_vector);
  ASSERT_EQ(1, converted_data_vector.size());
  ASSERT_EQ("123.12", converted_data_vector.at(0));
}
TEST(TypesConverterTest, DateToString) {
  std::vector<std::string> converted_data_vector;
  TypesConverter::ConvertDateValuesToString({123011}, converted_data_vector);
  ASSERT_EQ(1, converted_data_vector.size());
  ASSERT_EQ("0012-30-11", converted_data_vector.at(0));
}

}  // namespace