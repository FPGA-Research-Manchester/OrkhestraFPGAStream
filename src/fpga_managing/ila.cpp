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

#include "ila.hpp"

#include <iostream>

using namespace dbmstodspi::fpga_managing;

void ILA::StartILAs() {
  ILA::WriteToModule(0x10000000, 1);
  ILA::WriteToModule(0x10000004, 3);
  ILA::WriteToModule(0x12000000, 1);
  ILA::WriteToModule(0x12000004, 3);
  ILA::WriteToModule(0x14000000, 1);
}

void ILA::StartAxiILA() { ILA::WriteToModule(0x14000000, 1); }

auto ILA::GetValues(int clock_cycle, int location, ILADataTypes data_type)
    -> uint32_t {
  return ILA::ReadFromModule(
      ILA::CalcAddress(clock_cycle, location, static_cast<int>(data_type)));
}

auto ILA::CalcAddress(int clock, int ila_id, int offset) -> int {
  int base_address = 0;
  if (ila_id == 0) {
    base_address = 0x10000000;
  } else if (ila_id == 1) {
    base_address = 0x12000000;
  } else if (ila_id == 2) {
    base_address = 0x14000000;
  } else {
    throw std::runtime_error("Wrong ILA core ID given!");
  }
  return (base_address + (clock << 11) + (offset << 2));
}

void ILA::WriteToModule(int module_internal_address, uint32_t write_data) {
  volatile uint32_t* register_address =
      memory_manager_->GetVirtualRegisterAddress(module_internal_address);
  *register_address = write_data;
}

auto ILA::ReadFromModule(int module_internal_address) -> uint32_t {
  volatile uint32_t* register_address =
      memory_manager_->GetVirtualRegisterAddress(module_internal_address);
  return *register_address;
}

void ILA::PrintILAData(int ila_id, int max_clock) {
  for (int clock = 0; clock < max_clock; clock++) {
    std::cout << "ILA " << ila_id << " CLOCK " << clock << ":"
              << "CLOCK_CYCLE "
              << GetValues(clock, ila_id, ILADataTypes::kClockCycle) << "; "
              << "TYPE " << GetValues(clock, ila_id, ILADataTypes::kType)
              << "; "
              << "STREAMID "
              << GetValues(clock, ila_id, ILADataTypes::kStreamID) << "; "
              << "CHUNKID " << GetValues(clock, ila_id, ILADataTypes::kChunkID)
              << "; "
              << "STATE " << GetValues(clock, ila_id, ILADataTypes::kState)
              << "; "
              << "CHANNELID "
              << GetValues(clock, ila_id, ILADataTypes::kChannelID) << "; "
              << "LAST " << GetValues(clock, ila_id, ILADataTypes::kLast)
              << "; "
              << "DATA_15 "
              << GetValues(clock, ila_id, ILADataTypes::kDataAtPos15) << "; "
              << "INSTR_CHANNELID "
              << GetValues(clock, ila_id, ILADataTypes::kInstrChannelID) << "; "
              << "INSTR_PARAM "
              << GetValues(clock, ila_id, ILADataTypes::kInstrParam) << "; "
              << "INSTR_STREAMID "
              << GetValues(clock, ila_id, ILADataTypes::kInstrStreamID) << "; "
              << "INSTR_TYPE "
              << GetValues(clock, ila_id, ILADataTypes::kInstrType) << "; "
              << "JOIN_STATE "
              << GetValues(clock, ila_id, ILADataTypes::kJoinState) << "; "
              << std::endl;
  }
}

void ILA::PrintAxiILAData(int max_clock) {
  for (int clock = 0; clock < max_clock; clock++) {
    std::cout
        << "ILA " << 2 << " CLOCK " << clock << ":"
        << "kAxiClockCycle "
        << GetValues(clock, 2, ILADataTypes::kAxiClockCycle) << "; "
        << "kAxiAWV " << GetValues(clock, 2, ILADataTypes::kAxiAWV) << "; "
        << "kAxiAWR " << GetValues(clock, 2, ILADataTypes::kAxiAWR) << "; "
        << "kAxiAWA " << GetValues(clock, 2, ILADataTypes::kAxiAWA) << "; "
        << "kAxiWV " << GetValues(clock, 2, ILADataTypes::kAxiWV) << "; "
        << "kAxiWR " << GetValues(clock, 2, ILADataTypes::kAxiWR) << "; "
        << "kAxiWD " << GetValues(clock, 2, ILADataTypes::kAxiWD) << "; "
        << "kAxiWS " << GetValues(clock, 2, ILADataTypes::kAxiWS) << "; "
        << "kAxiBV " << GetValues(clock, 2, ILADataTypes::kAxiBV) << "; "
        << "kAxiBR " << GetValues(clock, 2, ILADataTypes::kAxiBR) << "; "
        << "kAxiBRESP " << GetValues(clock, 2, ILADataTypes::kAxiBRESP) << "; "
        << "kAxiARV " << GetValues(clock, 2, ILADataTypes::kAxiARV) << "; "
        << "kAxiARR " << GetValues(clock, 2, ILADataTypes::kAxiARR) << "; "
        << "kAxiARA " << GetValues(clock, 2, ILADataTypes::kAxiARA) << "; "
        << "kAxiRV " << GetValues(clock, 2, ILADataTypes::kAxiRV) << "; "
        << "kAxiRR " << GetValues(clock, 2, ILADataTypes::kAxiRR) << "; "
        << "kAxiRD " << GetValues(clock, 2, ILADataTypes::kAxiRD) << "; "
        << "kAxiRRESP " << GetValues(clock, 2, ILADataTypes::kAxiRRESP) << "; "
        << std::endl;
  }
}

void ILA::PrintDMAILAData(int max_clock) {
  for (int clock = 0; clock < max_clock; clock++) {
    std::cout
        << std::hex << "ILA " << 2 << " CLOCK " << clock << ":"
        << "IC_S " << GetValues(clock, 2, ILADataTypes::kIcS) << "; "
        << "IC_CH " << GetValues(clock, 2, ILADataTypes::kIcCh) << "; "
        << "IC_I " << GetValues(clock, 2, ILADataTypes::kIcI) << "; "
        << "IC_IP " << GetValues(clock, 2, ILADataTypes::kIcIp) << "; "
        << "IC_V " << GetValues(clock, 2, ILADataTypes::kIcV) << "; "
        << "IC_P " << GetValues(clock, 2, ILADataTypes::kIcP) << "; "
        << "ICD_BUSY " << GetValues(clock, 2, ILADataTypes::kIcdBusy) << "; "
        << "ICD_S " << GetValues(clock, 2, ILADataTypes::kIcdS) << "; "
        << "ICD_B " << GetValues(clock, 2, ILADataTypes::kIcdB) << "; "
        << "ICD_E " << GetValues(clock, 2, ILADataTypes::kIcdE) << "; "
        << "ICD_CH " << GetValues(clock, 2, ILADataTypes::kIcdCh) << "; "
        << "ICD_BU " << GetValues(clock, 2, ILADataTypes::kIcdBu) << "; "
        << "DR_A " << GetValues(clock, 2, ILADataTypes::kDrA) << "; "
        << "DR_L " << GetValues(clock, 2, ILADataTypes::kDrL) << "; "
        << "DR_S " << GetValues(clock, 2, ILADataTypes::kDrS) << "; "
        << "DR_B " << GetValues(clock, 2, ILADataTypes::kDrB) << "; "
        << "DR_E " << GetValues(clock, 2, ILADataTypes::kDrE) << "; "
        << "DR_CH " << GetValues(clock, 2, ILADataTypes::kDrCh) << "; "
        << "DR_BU " << GetValues(clock, 2, ILADataTypes::kDrBu) << "; "
        << "DR_AR " << GetValues(clock, 2, ILADataTypes::kDrAr) << "; "
        << "DR_AV " << GetValues(clock, 2, ILADataTypes::kDrAv) << "; "
        << "DRC_ARV " << GetValues(clock, 2, ILADataTypes::kDrcArv) << "; "
        << "DRC_ARB " << GetValues(clock, 2, ILADataTypes::kDrcArb) << "; "
        << "DRC_ARA " << GetValues(clock, 2, ILADataTypes::kDrcAra) << "; "
        << "DRC_ARL " << GetValues(clock, 2, ILADataTypes::kDrcArl) << "; "
        << "DRC_RD " << GetValues(clock, 2, ILADataTypes::kDrcRd) << "; "
        << "DRC_RV " << GetValues(clock, 2, ILADataTypes::kDrcRv) << "; "
        << "DRC_RR " << GetValues(clock, 2, ILADataTypes::kDrcRr) << "; "
        << "DRC_RL " << GetValues(clock, 2, ILADataTypes::kDrcRl) << "; "
        << "IB_D " << GetValues(clock, 2, ILADataTypes::kIbD) << "; "
        << "IB_V " << GetValues(clock, 2, ILADataTypes::kIbV) << "; "
        << "IB_S " << GetValues(clock, 2, ILADataTypes::kIbS) << "; "
        << "IB_B " << GetValues(clock, 2, ILADataTypes::kIbB) << "; "
        << "IB_CH " << GetValues(clock, 2, ILADataTypes::kIbCh) << "; "
        << "IB_CL " << GetValues(clock, 2, ILADataTypes::kIbCl) << "; "
        << "IB_L " << GetValues(clock, 2, ILADataTypes::kIbL) << "; "
        << "IB_SI " << GetValues(clock, 2, ILADataTypes::kIbSi) << "; "
        << "IB_VS " << GetValues(clock, 2, ILADataTypes::kIbVs) << "; "
        << "IB_BU " << GetValues(clock, 2, ILADataTypes::kIbBu) << "; "
        << "U_NA_DIN " << GetValues(clock, 2, ILADataTypes::kUNaDin) << "; "
        << "U_NA_DOUT " << GetValues(clock, 2, ILADataTypes::kUNaDout) << "; "
        << "U_NA_WADD " << GetValues(clock, 2, ILADataTypes::kUNaWadd) << "; "
        << "U_NA_RADD " << GetValues(clock, 2, ILADataTypes::kUNaRadd) << "; "
        << "U_NA_WEN " << GetValues(clock, 2, ILADataTypes::kUNaWen) << "; "
        << "U_RR_DIN " << GetValues(clock, 2, ILADataTypes::kURrDin) << "; "
        << "U_RR_DOUT " << GetValues(clock, 2, ILADataTypes::kURrDout) << "; "
        << "U_RR_WADD " << GetValues(clock, 2, ILADataTypes::kURrWadd) << "; "
        << "U_RR_RADD " << GetValues(clock, 2, ILADataTypes::kURrRadd) << "; "
        << "U_RR_WEN " << GetValues(clock, 2, ILADataTypes::kURrWen) << "; "
        << "U_RPB " << GetValues(clock, 2, ILADataTypes::kURpb) << "; "
        << "U_BS " << GetValues(clock, 2, ILADataTypes::kUBs) << "; "
        << std::endl;
  }
}