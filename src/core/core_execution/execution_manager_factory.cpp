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
#include "one_plan_node_scheduler.hpp"
#include "plan_evaluator.hpp"
#include "query_manager.hpp"
#include "schedule_state.hpp"

using orkhestrafs::core::core_execution::ExecutionManager;
using orkhestrafs::core::core_execution::ExecutionManagerFactory;
using orkhestrafs::core_interfaces::ExecutionManagerInterface;
using orkhestrafs::dbmstodspi::CSVReader;
using orkhestrafs::dbmstodspi::DataManager;
using orkhestrafs::dbmstodspi::ElasticResourceNodeScheduler;
using orkhestrafs::dbmstodspi::FPGADriverFactory;
using orkhestrafs::dbmstodspi::MemoryManager;
using orkhestrafs::dbmstodspi::OnePlanNodeScheduler;
using orkhestrafs::dbmstodspi::PlanEvaluator;
using orkhestrafs::dbmstodspi::QueryManager;
using orkhestrafs::dbmstodspi::ScheduleState;

auto ExecutionManagerFactory::GetManager(const Config& config)
    -> std::unique_ptr<ExecutionManagerInterface> {
  auto chosen_scheduler = std::make_unique<OnePlanNodeScheduler>();
  auto secondary_scheduler = std::make_unique<ElasticResourceNodeScheduler>(
      std::make_unique<PlanEvaluator>());

  return std::make_unique<ExecutionManager>(
      config, std::make_unique<QueryManager>(),
      std::make_unique<DataManager>(config.data_sizes, config.csv_separator,
                                    std::make_unique<CSVReader>()),
      std::make_unique<MemoryManager>(), std::make_unique<ScheduleState>(),
      std::make_unique<FPGADriverFactory>(), std::move(chosen_scheduler));
}