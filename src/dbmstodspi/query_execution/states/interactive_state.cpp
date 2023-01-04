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

#include "interactive_state.hpp"

#include <chrono>
#include <iostream>

#include "logger.hpp"
#include "schedule_state.hpp"
#include "sql_parser.hpp"
#include "sql_query_creator.hpp"
#include "orkhestra_exception.hpp"

using orkhestrafs::dbmstodspi::GraphProcessingFSMInterface;
using orkhestrafs::dbmstodspi::InteractiveState;
using orkhestrafs::dbmstodspi::ScheduleState;
using orkhestrafs::dbmstodspi::StateInterface;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;
using orkhestrafs::sql_parsing::SQLParser;
using orkhestrafs::sql_parsing::SQLQueryCreator;
using orkhestrafs::dbmstodspi::OrkhestraException;

auto InteractiveState::Execute(GraphProcessingFSMInterface* fsm)
    -> std::unique_ptr<StateInterface> {
  Log(LogLevel::kTrace, "In interactive state");

  switch (GetOption(fsm)) {
    case 1:
      fsm->PrintHWState();
      break;
    case 2:
      if (fsm->GetFPGASpeed() == 100) {
        fsm->SetClockSpeed(300);
      } else {
        fsm->SetClockSpeed(100);
      }
      fsm->LoadStaticBitstream();
      break;
    case 3:
      std::cout << "Enter new time limit in seconds: " << std::endl;
      fsm->ChangeSchedulingTimeLimit(GetDouble());
      break;
    case 4:
      std::cout << "Enter new time limit in seconds: " << std::endl;
      fsm->ChangeExecutionTimeLimit(GetInteger());
      break;
    case 5: {
      auto [db, file, is_postgres] = GetQueryFromInput();
      try {
        auto begin = std::chrono::steady_clock::now();
        fsm->AddNewNodes(GetExecutionPlanFile(db, file, is_postgres));
        auto end = std::chrono::steady_clock::now();
        long planning =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                .count();
        std::cout << "PLANNING: " << planning << " ms" << std::endl;
        fsm->SetStartTimer();
        return std::make_unique<ScheduleState>();
      } catch (OrkhestraException exception) {
        if (fsm->IsSWBackupEnabled()) {
          SQLParser::PrintResults(std::move(file), std::move(db), is_postgres);
        } else {
          throw std::runtime_error("Python call unsuccessful!");
        }
      }
      
      break;
    }
    case 6:
      fsm->SetFinishedFlag();
      break;
    case 7:
      fsm->LoadStaticBitstream();
      break;
    case 8: {
      auto new_bitstream = GetBitstreamToLoad(fsm->GetCurrentHW());
      if (new_bitstream.operation_type != QueryOperationType::kPassThrough) {
        fsm->LoadBitstream(new_bitstream);
      }
      break;
    }
    case 9:
      if (fsm->IsHWPrintEnabled()){
        fsm->SetHWPrint(false);
      } else {
        fsm->SetHWPrint(true);
      }
      break;
    default:
      std::cout << "Incorrect option" << std::endl;
  }

  return std::make_unique<InteractiveState>();
}

void InteractiveState::PrintOutGivenOptions(
    const std::vector<std::string> list_of_options) {
  std::cout << "0: Go back" << std::endl;
  for (int option_i = 0; option_i < list_of_options.size(); option_i++) {
    std::cout << option_i + 1 << ": " << list_of_options.at(option_i)
              << std::endl;
  }
}

auto InteractiveState::GetBitstreamToLoad(
    const std::map<QueryOperationType, OperationPRModules> bitstream_map)
    -> ScheduledModule {
  bool bitstream_choice_complete = false;
  ScheduledModule chosen_bitstream = {
      "", QueryOperationType::kPassThrough, "", {0, 0}, false};
  bool operation_chosen = false;
  QueryOperationType current_operation = QueryOperationType::kAddition;
  std::vector<QueryOperationType> list_of_operations;
  std::vector<std::string> list_of_operation_strings;
  std::vector<std::string> list_of_bitstreams;
  for (const auto& [op_type, data] : bitstream_map) {
    list_of_operation_strings.push_back(operation_names_.at(op_type));
    list_of_operations.push_back(op_type);
  }
  while (!bitstream_choice_complete) {
    if (operation_chosen) {
      std::cout << "Choose bitstream: " << std::endl;
      PrintOutGivenOptions(list_of_bitstreams);
      auto answer = GetInteger();
      if (answer == 0) {
        operation_chosen = false;
        list_of_bitstreams.clear();
      } else {
        bitstream_choice_complete = true;
        std::string chosen_bitstream_name = list_of_bitstreams.at(answer - 1);
        chosen_bitstream = {"",
                            current_operation,
                            chosen_bitstream_name,
                            {bitstream_map.at(current_operation)
                                 .bitstream_map.at(chosen_bitstream_name)
                                 .fitting_locations.front(),
                             bitstream_map.at(current_operation)
                                     .bitstream_map.at(chosen_bitstream_name)
                                     .fitting_locations.front() +
                                 bitstream_map.at(current_operation)
                                     .bitstream_map.at(chosen_bitstream_name)
                                     .length - 1}};
      }
    } else {
      std::cout << "Choose operation: " << std::endl;
      PrintOutGivenOptions(list_of_operation_strings);
      auto answer = GetInteger();
      if (answer == 0) {
        bitstream_choice_complete = true;
      } else {
        current_operation = list_of_operations.at(answer - 1);
        operation_chosen = true;
        for (const auto& [bitstream, data] :
             bitstream_map.at(current_operation).bitstream_map) {
          list_of_bitstreams.push_back(bitstream);
        }
      }
    }
  }
  return chosen_bitstream;
}

auto InteractiveState::GetQueryFromInput()
    -> std::tuple<std::vector<std::string>, std::string, bool> {
  auto is_postgre = IsPostgreSQL();
  std::vector<std::string> db;
  if (is_postgre) {
    std::cout << "Enter DB name:" << std::endl;
    auto database = GetStdInput();
    database.pop_back();
    db.emplace_back(database);
  } else {
    std::cout << "Enter Schema name:" << std::endl;
    auto input = GetStdInput();
    input.pop_back();
    db.emplace_back(input);
    std::cout << "Enter Catalog name:" << std::endl;
    input = GetStdInput();
    input.pop_back();
    db.emplace_back(input);
  }
  std::cout << "Enter filename with query:" << std::endl;
  auto query_filename = GetStdInput();
  query_filename.pop_back();
  return {std::move(db), std::move(query_filename), is_postgre};
}

auto InteractiveState::IsPostgreSQL() -> bool { 
    std::cout << std::endl;
    std::cout << "Which DBMS would you like to use?" << std::endl;
    std::cout << "1: PostgreSQL" << std::endl;
    std::cout << "2: Presto" << std::endl;
    return GetInteger() == 1;
}

auto InteractiveState::GetExecutionPlanFile(const std::vector<std::string> db,
                                            const std::string file,
                                            bool is_postgres)
    -> std::string {
  SQLQueryCreator sql_creator;
  SQLParser::CreatePlan(sql_creator, file, db, is_postgres);
  return sql_creator.ExportInputDef();
}

void InteractiveState::PrintOptions(GraphProcessingFSMInterface* fsm) {
  std::cout << std::endl;
  std::cout << "===========MENU==========" << std::endl;
  std::cout << "Which operation would you like to do?" << std::endl;
  std::cout << "1: Print HW state" << std::endl;
  if (fsm->GetFPGASpeed() == 300) {
    std::cout << "2: Change FPGA speed (currently MAX - 300 MHz)" << std::endl;
  } else if (fsm->GetFPGASpeed() == 100) {
    std::cout << "2: Change FPGA speed (currently 100 MHz)" << std::endl;
  } else {
    throw std::runtime_error("Unsupported clock speed!");
  }
  std::cout << "3: Change scheduling time limit" << std::endl;
  std::cout << "4: Change execution time limit" << std::endl;
  std::cout << "5: Run SQL" << std::endl;
  std::cout << "6: EXIT" << std::endl;
  std::cout << "7: Reset" << std::endl;
  std::cout << "8: Load module" << std::endl;
  if (fsm->IsHWPrintEnabled()) {
    std::cout << "9: Disable HW print" << std::endl;
  } else {
    std::cout << "9: Enable HW print" << std::endl;
  }
  std::cout << "Choose one of the supported options by typing a valid number "
               "and a ';'"
            << std::endl;
}

auto InteractiveState::GetOption(GraphProcessingFSMInterface* fsm) -> int {
  PrintOptions(fsm);
  return GetInteger();
}

auto InteractiveState::GetInteger() -> int {
  std::string answer = "";
  bool is_number = false;
  while (!is_number) {
    answer = GetStdInput();
    answer.pop_back();
    is_number = std::all_of(answer.begin(), answer.end(),
                            [](auto digit) { return isdigit(digit); });
    if (!is_number) {
      std::cout << "Invalid input" << std::endl;
    }
  }
  return std::stoi(answer);
}

auto InteractiveState::GetDouble() -> double {
  std::string answer = "";
  bool is_number = false;
  while (!is_number) {
    answer = GetStdInput();
    answer.pop_back();
    is_number = std::all_of(answer.begin(), answer.end(), [](auto digit) {
      return isdigit(digit) || digit == '.';
    });
    if (!is_number) {
      std::cout << "Invalid input" << std::endl;
    }
  }
  return std::stod(answer);
}

auto InteractiveState::GetStdInput() -> std::string {
  std::string current_input = "";
  std::cout << "> ";
  std::cin >> current_input;
  while (current_input.back() != ';') {
    std::string more_current_input = "";
    std::cout << "> ";
    std::cin >> more_current_input;
    current_input += more_current_input;
  }
  std::cout << std::endl;
  return current_input;
}
// Lazily put option functions into this class rather than the FSM or even
// better a separate class