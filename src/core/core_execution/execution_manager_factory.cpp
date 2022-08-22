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

#include "execution_manager_factory.hpp"

#include "csv_reader.hpp"
#include "data_manager.hpp"
#include "elastic_resource_scheduler.hpp"
#include "execution_manager.hpp"
#include "fpga_driver_factory.hpp"
#include "memory_manager.hpp"
#include "plan_evaluator.hpp"
#include "query_manager.hpp"
#include "rapidjson_reader.hpp"
#include "schedule_state.hpp"
#include "setup_benchmark_schedule_state.hpp"
#include "setup_scheduling_state.hpp"
#include "load_tables_state.hpp"

using orkhestrafs::core::core_execution::ExecutionManager;
using orkhestrafs::core::core_execution::ExecutionManagerFactory;
using orkhestrafs::core_interfaces::ExecutionManagerInterface;
using orkhestrafs::dbmstodspi::CSVReader;
using orkhestrafs::dbmstodspi::DataManager;
using orkhestrafs::dbmstodspi::ElasticResourceNodeScheduler;
using orkhestrafs::dbmstodspi::FPGADriverFactory;
using orkhestrafs::dbmstodspi::MemoryManager;
using orkhestrafs::dbmstodspi::PlanEvaluator;
using orkhestrafs::dbmstodspi::QueryManager;
using orkhestrafs::dbmstodspi::RapidJSONReader;
using orkhestrafs::dbmstodspi::SetupSchedulingState;
using orkhestrafs::dbmstodspi::SetupBenchmarkScheduleState;
using orkhestrafs::dbmstodspi::LoadTablesState;

auto ExecutionManagerFactory::GetManager(const Config& config, bool is_interactive)
    -> std::unique_ptr<ExecutionManagerInterface> {
  auto scheduler = std::make_unique<ElasticResourceNodeScheduler>(
      std::make_unique<PlanEvaluator>());

  std::unique_ptr<StateInterface> start_state;
  if (is_interactive) {
    start_state = std::make_unique<LoadTablesState>();
  } else {
      // TODO: Make this more robust - Throw an error if interactive & benchmarking
    if (config.benchmark_scheduler) {
      start_state = std::make_unique<SetupBenchmarkScheduleState>();
    } else {
      start_state = std::make_unique<SetupSchedulingState>();
    }
  }

  return std::make_unique<ExecutionManager>(
      config,
      std::make_unique<QueryManager>(std::make_unique<RapidJSONReader>()),
      std::make_unique<DataManager>(config.data_sizes, config.csv_separator,
                                    std::make_unique<CSVReader>()),
      std::make_unique<MemoryManager>(), std::move(start_state),
      std::make_unique<FPGADriverFactory>(), std::move(scheduler));
}