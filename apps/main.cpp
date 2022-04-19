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

#include <chrono>
#include <iostream>
#include <string>
#include <utility>

#include "core.hpp"
#include "cxxopts.hpp"
#include "logger.hpp"
#include "sql_query_creator.hpp"
#include "table_data.hpp"
#include "sql_query_data.hpp"

using namespace std;
using orkhestrafs::core::Core;
using orkhestrafs::core_interfaces::table_data::ColumnDataType;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;
using orkhestrafs::dbmstodspi::logging::SetLoggingLevel;
using orkhestrafs::sql_parsing::query_data::CompareFunctions;
using orkhestrafs::sql_parsing::SQLQueryCreator;
using orkhestrafs::sql_parsing::query_data::TableColumn;

/**
 * @brief Helper method to run the given query nodes and their subsequent nodes
 * while measuring and printing the overall time it took to process the queries.
 *
 * This includes data writing and reading from and to the DDR.
 * @param input_def_filename Filename for the JSON file defining the input query
 * graph.
 * @param config_filename Filename for the INI file containing paths to query
 * configuration files.
 */
void MeasureOverallTime(string input_def_filename, string config_filename) {
  auto begin = chrono::steady_clock::now();
  Core::Run(std::move(input_def_filename), std::move(config_filename));
  auto end = chrono::steady_clock::now();
  std::cout << "TOTAL RUNTIME:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
  Log(LogLevel::kInfo,
      "Overall time = " +
          to_string(
              chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                  .count()) +
          "[ms]");
}

auto CreateLineitemTable(SQLQueryCreator* sql_creator, int row_count,
                         std::string filename) -> std::string {
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
  return sql_creator->RegisterTable(filename, columns, row_count);
}

auto CreatePartTable(SQLQueryCreator* sql_creator, int row_count,
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
  return sql_creator->RegisterTable(filename, columns, row_count);
}

void ConfigureQ19FirstFilter(SQLQueryCreator& sql_creator,
                             string& first_filter) {
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
void ConfigureQ19SecondFilter(SQLQueryCreator& sql_creator,
                              string& second_filter) {
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
void CreateQ19TPCH(SQLQueryCreator& sql_creator) {
  auto lineitem_table =
      CreateLineitemTable(&sql_creator, 6001215, "lineitem1.csv");
  auto first_filter = sql_creator.RegisterFilter(lineitem_table);
  ConfigureQ19FirstFilter(sql_creator, first_filter);
  auto part_table = CreatePartTable(&sql_creator, 200000, "part1.csv");
  auto join = sql_creator.RegisterJoin(first_filter, "L_PARTKEY", part_table,
                                       "P_PARTKEY");
  auto second_filter = sql_creator.RegisterFilter(join);
  ConfigureQ19SecondFilter(sql_creator, second_filter);
  auto addition = sql_creator.RegisterAddition(second_filter, "L_DISCOUNT", true, 1);
  auto multiplication = sql_creator.RegisterMultiplication(
      addition, "L_DISCOUNT", "L_EXTENDEDPRICE", "TEMP_MUL");
  sql_creator.RegisterAggregation(multiplication, "TEMP_MUL");
}
void RunCodedQuery() {
  const auto default_config_filename = "fast_benchmark_config.ini";
  SQLQueryCreator sql_creator;
  CreateQ19TPCH(sql_creator);
  auto begin = chrono::steady_clock::now();
  Core::Run(std::move(sql_creator.ExportInputDef()),
            std::move(default_config_filename));
  auto end = chrono::steady_clock::now();
  std::cout << "TOTAL RUNTIME:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
  Log(LogLevel::kInfo,
      "Overall time = " +
          to_string(
              chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                  .count()) +
          "[ms]");
}

/**
 * @brief Main method of the program.
 *
 * Creates query nodes to be processed and runs the
 * query nodes according to the given config.
 */
auto main(int argc, char* argv[]) -> int {
  cxxopts::Options options(
      "OrkhestraFPGAStream",
      "Accelerate the given query operations with an available FPGA!");

  options.add_options()("i,input", "Input definition",
                        cxxopts::value<std::string>())(
      "c,config", "Config file for used hardware",
      cxxopts::value<std::string>())("v,verbose", "Additional debug messages")(
      "t,trace", "Enable all trace signals")("q,quiet", "Disable all logging")(
      "h,help", "Print usage")("r,run", "Run hardcoded query");

  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    cout << options.help() << endl;
    exit(0);
  }

  if (result.count("quiet")) {
    SetLoggingLevel(LogLevel::kOff);
  } else if (result.count("trace")) {
    SetLoggingLevel(LogLevel::kTrace);
  } else if (result.count("verbose")) {
    SetLoggingLevel(LogLevel::kDebug);
  } else {
    SetLoggingLevel(LogLevel::kInfo);
  }

  if (result.count("run")) {
    RunCodedQuery();
  } else {
    MeasureOverallTime(result["input"].as<string>(),
                       result["config"].as<string>());
  }

  return 0;
}