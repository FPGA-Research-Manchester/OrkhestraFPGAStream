/*
Copyright 2022 University of Manchester

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

#include "q19_creator.hpp"

#include <utility>

using orkhestrafs::sql_parsing::Q19Creator;

void Q19Creator::CreateQ19TPCH(SQLQueryCreator& sql_creator) {
  auto lineitem_table =
      CreateLineitemTable(&sql_creator, 6001215, "lineitem1.csv");
  auto first_filter = sql_creator.RegisterFilter(lineitem_table);
  ConfigureQ19FirstFilter(sql_creator, first_filter);
  auto part_table = CreatePartTable(&sql_creator, 200000, "part1.csv");
  auto join = sql_creator.RegisterJoin(first_filter, "L_PARTKEY", part_table,
                                       "P_PARTKEY");
  auto second_filter = sql_creator.RegisterFilter(join);
  ConfigureQ19SecondFilter(sql_creator, second_filter);
  auto addition =
      sql_creator.RegisterAddition(second_filter, "L_DISCOUNT", true, 1);
  auto multiplication = sql_creator.RegisterMultiplication(
      addition, "L_EXTENDEDPRICE", "L_DISCOUNT", "TEMP_MUL");
  sql_creator.RegisterAggregation(multiplication, "TEMP_MUL");
}

auto Q19Creator::CreateLineitemTable(SQLQueryCreator* sql_creator,
                                     int row_count, std::string filename)
    -> std::string {
  std::vector<TableColumn> columns;
  columns.emplace_back(ColumnDataType::kInteger, 1, "L_ORDERKEY");
  columns.emplace_back(ColumnDataType::kInteger, 1, "L_PARTKEY");
  columns.emplace_back(ColumnDataType::kInteger, 1, "L_SUPPKEY");
  columns.emplace_back(ColumnDataType::kInteger, 1, "L_LINENUMBER");
  columns.emplace_back(ColumnDataType::kDecimal, 1, "L_QUANTITY");
  columns.emplace_back(ColumnDataType::kDecimal, 1, "L_EXTENDEDPRICE");
  columns.emplace_back(ColumnDataType::kDecimal, 1, "L_DISCOUNT");
  columns.emplace_back(ColumnDataType::kDecimal, 1, "L_TAX");
  columns.emplace_back(ColumnDataType::kVarchar, 1, "L_RETURNFLAG");
  columns.emplace_back(ColumnDataType::kVarchar, 1, "L_LINESTATUS");
  columns.emplace_back(ColumnDataType::kDate, 1, "L_SHIPDATE");
  columns.emplace_back(ColumnDataType::kDate, 1, "L_COMMITDATE");
  columns.emplace_back(ColumnDataType::kDate, 1, "L_RECEIPTDATE");
  columns.emplace_back(ColumnDataType::kVarchar, 25, "L_SHIPINSTRUCT");
  columns.emplace_back(ColumnDataType::kVarchar, 10, "L_SHIPMODE");
  columns.emplace_back(ColumnDataType::kVarchar, 44, "L_COMMENT");
  return sql_creator->RegisterTable(std::move(filename), columns, row_count);
}

auto Q19Creator::CreatePartTable(SQLQueryCreator* sql_creator, int row_count,
                                 std::string filename) -> std::string {
  std::vector<TableColumn> columns;
  columns.emplace_back(ColumnDataType::kInteger, 1, "P_PARTKEY");
  columns.emplace_back(ColumnDataType::kVarchar, 55, "P_NAME");
  columns.emplace_back(ColumnDataType::kVarchar, 25, "P_MFGR");
  columns.emplace_back(ColumnDataType::kVarchar, 10, "P_BRAND");
  columns.emplace_back(ColumnDataType::kVarchar, 25, "P_TYPE");
  columns.emplace_back(ColumnDataType::kInteger, 1, "P_SIZE");
  columns.emplace_back(ColumnDataType::kVarchar, 10, "P_CONTAINER");
  columns.emplace_back(ColumnDataType::kDecimal, 1, "P_RETAILPRICE");
  columns.emplace_back(ColumnDataType::kVarchar, 23, "P_COMMENT");
  return sql_creator->RegisterTable(std::move(filename), columns, row_count);
}

void Q19Creator::ConfigureQ19FirstFilter(SQLQueryCreator& sql_creator,
                                         std::string& first_filter) {
  auto quantity_big = sql_creator.AddDoubleComparison(
      first_filter, "L_QUANTITY", CompareFunctions::kLessThanOrEqual, 30.0);
  auto quantity_small = sql_creator.AddDoubleComparison(
      first_filter, "L_QUANTITY", CompareFunctions::kGreaterThanOrEqual, 1.0);
  auto ship_air_reg = sql_creator.AddStringComparison(
      first_filter, "L_SHIPMODE", CompareFunctions::kEqual, "AIR REG");
  auto ship_air = sql_creator.AddStringComparison(
      first_filter, "L_SHIPMODE", CompareFunctions::kEqual, "AIR");
  auto deliver_in_person = sql_creator.AddStringComparison(
      first_filter, "L_SHIPINSTRUCT", CompareFunctions::kEqual,
      "DELIVER IN PERSON");
  auto shipmode = sql_creator.AddOr(first_filter, {ship_air, ship_air_reg});
  sql_creator.AddAnd(first_filter, {shipmode, deliver_in_person, quantity_small,
                                    quantity_big});
}
void Q19Creator::ConfigureQ19SecondFilter(SQLQueryCreator& sql_creator,
                                          std::string& second_filter) {
  auto second_quantity_big_less = sql_creator.AddDoubleComparison(
      second_filter, "L_QUANTITY", CompareFunctions::kLessThanOrEqual, 30.0);
  auto second_quantity_med_less = sql_creator.AddDoubleComparison(
      second_filter, "L_QUANTITY", CompareFunctions::kLessThanOrEqual, 20.0);
  auto second_quantity_small_less = sql_creator.AddDoubleComparison(
      second_filter, "L_QUANTITY", CompareFunctions::kLessThanOrEqual, 11.0);
  auto second_quantity_big_more = sql_creator.AddDoubleComparison(
      second_filter, "L_QUANTITY", CompareFunctions::kGreaterThanOrEqual, 20.0);
  auto second_quantity_med_more = sql_creator.AddDoubleComparison(
      second_filter, "L_QUANTITY", CompareFunctions::kGreaterThanOrEqual, 10.0);
  auto second_quantity_small_more = sql_creator.AddDoubleComparison(
      second_filter, "L_QUANTITY", CompareFunctions::kGreaterThanOrEqual, 1.0);
  auto brand1 = sql_creator.AddStringComparison(
      second_filter, "P_BRAND", CompareFunctions::kEqual, "Brand#12");
  auto brand2 = sql_creator.AddStringComparison(
      second_filter, "P_BRAND", CompareFunctions::kEqual, "Brand#23");
  auto brand3 = sql_creator.AddStringComparison(
      second_filter, "P_BRAND", CompareFunctions::kEqual, "Brand#34");
  auto size_low = sql_creator.AddIntegerComparison(
      second_filter, "P_SIZE", CompareFunctions::kGreaterThanOrEqual, 1);
  auto size_small = sql_creator.AddIntegerComparison(
      second_filter, "P_SIZE", CompareFunctions::kLessThanOrEqual, 5);
  auto size_med = sql_creator.AddIntegerComparison(
      second_filter, "P_SIZE", CompareFunctions::kGreaterThanOrEqual, 10);
  auto size_big = sql_creator.AddIntegerComparison(
      second_filter, "P_SIZE", CompareFunctions::kGreaterThanOrEqual, 15);
  auto container_small_case = sql_creator.AddStringComparison(
      second_filter, "P_CONTAINER", CompareFunctions::kEqual, "SM CASE");
  auto container_small_box = sql_creator.AddStringComparison(
      second_filter, "P_CONTAINER", CompareFunctions::kEqual, "SM BOX");
  auto container_small_pack = sql_creator.AddStringComparison(
      second_filter, "P_CONTAINER", CompareFunctions::kEqual, "SM PACK");
  auto container_small_pkg = sql_creator.AddStringComparison(
      second_filter, "P_CONTAINER", CompareFunctions::kEqual, "SM PKG");
  auto container_med_case = sql_creator.AddStringComparison(
      second_filter, "P_CONTAINER", CompareFunctions::kEqual, "MED CASE");
  auto container_med_box = sql_creator.AddStringComparison(
      second_filter, "P_CONTAINER", CompareFunctions::kEqual, "MED BOX");
  auto container_med_pack = sql_creator.AddStringComparison(
      second_filter, "P_CONTAINER", CompareFunctions::kEqual, "MED PACK");
  auto container_med_pkg = sql_creator.AddStringComparison(
      second_filter, "P_CONTAINER", CompareFunctions::kEqual, "MED PKG");
  auto container_big_case = sql_creator.AddStringComparison(
      second_filter, "P_CONTAINER", CompareFunctions::kEqual, "LG CASE");
  auto container_big_box = sql_creator.AddStringComparison(
      second_filter, "P_CONTAINER", CompareFunctions::kEqual, "LG BOX");
  auto container_big_pack = sql_creator.AddStringComparison(
      second_filter, "P_CONTAINER", CompareFunctions::kEqual, "LG PACK");
  auto container_big_pkg = sql_creator.AddStringComparison(
      second_filter, "P_CONTAINER", CompareFunctions::kEqual, "LG PKG");
  auto container_big =
      sql_creator.AddOr(second_filter, {container_big_pkg, container_big_pack,
                                        container_big_box, container_big_case});
  auto container_med =
      sql_creator.AddOr(second_filter, {container_med_pkg, container_med_pack,
                                        container_med_box, container_med_case});
  auto container_small = sql_creator.AddOr(
      second_filter, {container_small_pkg, container_small_pack,
                      container_small_box, container_small_case});
  auto small = sql_creator.AddAnd(
      second_filter, {container_small, size_small, brand1,
                      second_quantity_small_more, second_quantity_small_less});
  auto med = sql_creator.AddAnd(
      second_filter, {container_med, size_med, brand2, second_quantity_med_more,
                      second_quantity_med_less});
  auto big = sql_creator.AddAnd(
      second_filter, {container_big, size_big, brand3, second_quantity_big_more,
                      second_quantity_big_less});
  auto or_filter = sql_creator.AddOr(second_filter, {small, med, big});
  sql_creator.AddAnd(second_filter, {size_low, or_filter});
}