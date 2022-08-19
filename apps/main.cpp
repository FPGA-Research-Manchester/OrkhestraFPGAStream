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
#include "q19_creator.hpp"
#include "sql_parser.hpp"
#include "sql_query_creator.hpp"
#include "sql_query_data.hpp"
#include "table_data.hpp"

using namespace std;
using orkhestrafs::core::Core;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;
using orkhestrafs::dbmstodspi::logging::SetLoggingLevel;
using orkhestrafs::sql_parsing::Q19Creator;
using orkhestrafs::sql_parsing::SQLParser;
using orkhestrafs::sql_parsing::SQLQueryCreator;

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
void MeasureOverallTimeOfParsedPlan(string input_def_filename,
                                    string config_filename) {
  auto begin = chrono::steady_clock::now();
  Core::Run(std::move(input_def_filename), std::move(config_filename));
  auto end = chrono::steady_clock::now();
  /*std::cout << "TOTAL RUNTIME INCLUDING CONFIG READING:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;*/
  Log(LogLevel::kInfo,
      "Overall time = " +
          to_string(
              chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                  .count()) +
          "[ms]");
}

// Storing queries inside files for fun.
void RunSQLQuery(string query_filename, string config_filename,
                 string database = "tpch_001") {
  auto frontend_begin = chrono::steady_clock::now();
  SQLQueryCreator sql_creator;
  SQLParser::CreatePlan(sql_creator, std::move(query_filename),
                        std::move(database));
  auto frontend_end = chrono::steady_clock::now();
  std::cout << "PARSING RUNTIME: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   frontend_end - frontend_begin)
                   .count()
            << std::endl;
  auto exec_begin = chrono::steady_clock::now();
  Core::Run(std::move(sql_creator.ExportInputDef()),
            std::move(config_filename));
  auto exec_end = chrono::steady_clock::now();
  /*std::cout << "EXECUTION RUNTIME: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(exec_end -
                                                                     exec_begin)
                   .count()
            << std::endl;*/
  Log(LogLevel::kInfo,
      "Execution time = " +
          to_string(chrono::duration_cast<std::chrono::milliseconds>(exec_end -
                                                                     exec_begin)
                  .count()) +
          "[ms]");
}

void RunCodedQuery(string config_filename) {
  SQLQueryCreator sql_creator;
  Q19Creator::CreateQ19TPCH(sql_creator);
  auto begin = chrono::steady_clock::now();
  Core::Run(std::move(sql_creator.ExportInputDef()),
            std::move(config_filename));
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
      "c,config", "Config file for additional options",
      cxxopts::value<std::string>())("v,verbose", "Additional debug messages")(
      "t,trace", "Enable all trace signals")("q,quiet", "Disable all logging")(
      "h,help", "Print usage")(
      "r,run",
      "Run SQL query provided in the file given. Or type 'example' for Q19.",
      cxxopts::value<std::string>())(
      "d,database",
      "Specify PostgreSQL database for running SQL queries. Default database is tpch_001",
      cxxopts::value<std::string>());

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

  string command = "./OrkhestraFPGAStream";
  for (const auto key_value : result.arguments()) {
    command += " --" + key_value.key() + " " + key_value.value();
  }
  Log(LogLevel::kDebug, command);

  string config_name = "default_config.ini";
  if (result.count("config")) {
    config_name = result["config"].as<string>();
  } else {
    cout << "Using default config!" << endl;
  }

  if ((result.count("run")) && (result.count("input"))) {
    throw runtime_error("Please give only a parsed input or an SQL input");
  }
  if (!(result.count("run") || result.count("input"))) {
    throw runtime_error("Please give one of the required inputs: SQL/parsed!");
  }

  if (result.count("run")) {
    if (result["run"].as<string>() == "example") {
      cout << "Executing default Q19 example!" << endl;
      RunCodedQuery(config_name);
    } else {
      if (result.count("database")) {
        RunSQLQuery(result["run"].as<string>(), config_name,
                    result["database"].as<string>());
      } else {
        RunSQLQuery(result["run"].as<string>(), config_name);
      }
    }
  } else {
    if (result.count("database")) {
      throw runtime_error("Database specification is not required when execution premade plans!");
    }
    MeasureOverallTimeOfParsedPlan(result["input"].as<string>(), config_name);
  }

  return 0;
}