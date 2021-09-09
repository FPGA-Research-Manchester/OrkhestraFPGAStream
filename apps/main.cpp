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

#include <chrono>
#include <iostream>
#include <string>
#include <utility>

#include "core.hpp"
#include "cxxopts.hpp"
#include "logger.hpp"

using namespace std;
using orkhestrafs::core::Core;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;
using orkhestrafs::dbmstodspi::logging::SetLoggingLevel;

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
      "h,help", "Print usage");

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

  MeasureOverallTime(result["input"].as<string>(),
                     result["config"].as<string>());

  return 0;
}