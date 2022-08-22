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

#include <iostream>

#include "logger.hpp"
#include "schedule_state.hpp"

using orkhestrafs::dbmstodspi::GraphProcessingFSMInterface;
using orkhestrafs::dbmstodspi::ScheduleState;
using orkhestrafs::dbmstodspi::InteractiveState;
using orkhestrafs::dbmstodspi::StateInterface;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

auto InteractiveState::Execute(GraphProcessingFSMInterface* fsm)
    -> std::unique_ptr<StateInterface> {
  Log(LogLevel::kTrace, "In interactive state");

  switch(GetOption(fsm)) {
    case 1:
      fsm->PrintHWState();
      break;
    case 2:
      if (fsm->GetFPGASpeed()==100){
        fsm->SetClockSpeed(300);
      } else{
        fsm->SetClockSpeed(100);
      }
      fsm->LoadStaticBitstream();
      break;
    case 3:
      std::cout<<"Enter new time limit in seconds: "<<std::endl;
      fsm->ChangeSchedulingTimeLimit(GetDouble());
      break;
    case 4:
      std::cout<<"Enter new time limit in seconds: "<<std::endl;
      fsm->ChangeExecutionTimeLimit(GetInteger());
      break;
    case 5:
      throw std::runtime_error("Not implemented");
      break;
    default:
      std::cout<<"Incorrect option"<<std::endl;
  }

  return std::make_unique<InteractiveState>();
}

void InteractiveState::PrintOptions(GraphProcessingFSMInterface* fsm){
  std::cout << "Which operation would you like to do?" << std::endl;
  std::cout << "1: Print HW state"<<std::endl;
  if (fsm->GetFPGASpeed() == 300){
    std::cout << "2: Change FPGA speed to 100 MHz"<<std::endl;
  } else if (fsm->GetFPGASpeed() == 100){
    std::cout << "2: Change FPGA speed to 300 MHz"<<std::endl;
  } else {
    throw std::runtime_error("Unsupported clock speed!");
  }
  std::cout << "3: Change scheduling time limit"<<std::endl;
  std::cout << "4: Change execution time limit"<<std::endl;
  std::cout << "5: Run SQL"<<std::endl;
  std::cout << "Choose one of the supported options by typing a valid number and a ';'" << std::endl;
}

auto InteractiveState::GetOption(GraphProcessingFSMInterface* fsm) -> int {
    PrintOptions(fsm);
    return GetInteger();
}

auto InteractiveState::GetInteger() -> int{
  std::string answer = "";
  bool is_number = false;
  while (!is_number){
    answer = GetStdInput();
    answer.pop_back();
    is_number = std::all_of(answer.begin(), answer.end(), [](auto digit){return isdigit(digit);});
    if (!is_number){
      std::cout<<"Invalid input"<<std::endl;
    }
  }
  return std::stoi(answer);
}

auto InteractiveState::GetDouble() -> double{
  std::string answer = "";
  bool is_number = false;
  while (!is_number){
    answer = GetStdInput();
    answer.pop_back();
    is_number = std::all_of(answer.begin(), answer.end(), [](auto digit){return isdigit(digit) || digit == '.';});
    if (!is_number){
      std::cout<<"Invalid input"<<std::endl;
    }
  }
  return std::stod(answer);
}

auto InteractiveState::GetStdInput() -> std::string {
  std::string current_input = "";
  std::cin >> current_input;
  while (current_input.back() != ';'){
    std::string more_current_input = "";
    std::cin >> more_current_input;
    current_input += more_current_input;
  }
  return current_input;
}
// Lazily put option functions into this class rather than the FSM or even better a separate class