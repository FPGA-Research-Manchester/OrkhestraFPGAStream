#include "ila.hpp"

#include <iostream>

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
  if (ila_id == 0) {
    return (0x10000000 + (clock << 11) + (offset << 2));
  } else if (ila_id == 1) {
    return (0x12000000 + (clock << 11) + (offset << 2));
  } else if (ila_id == 2) {
    return (0x14000000 + (clock << 11) + (offset << 2));
  } else {
    throw std::runtime_error("Wrong ILA core ID given!");
  }
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
    std::cout << std::hex << "ILA " << 2 << " CLOCK " << clock << ":"
              << "IC_S " << GetValues(clock, 2, ILADataTypes::IC_S) << "; "
              << "IC_CH " << GetValues(clock, 2, ILADataTypes::IC_CH) << "; "
              << "IC_I " << GetValues(clock, 2, ILADataTypes::IC_I) << "; "
              << "IC_IP " << GetValues(clock, 2, ILADataTypes::IC_IP) << "; "
              << "IC_V " << GetValues(clock, 2, ILADataTypes::IC_V) << "; "
              << "IC_P " << GetValues(clock, 2, ILADataTypes::IC_P) << "; "
              << "ICD_BUSY " << GetValues(clock, 2, ILADataTypes::ICD_BUSY)
              << "; "
              << "ICD_S " << GetValues(clock, 2, ILADataTypes::ICD_S) << "; "
              << "ICD_B " << GetValues(clock, 2, ILADataTypes::ICD_B) << "; "
              << "ICD_E " << GetValues(clock, 2, ILADataTypes::ICD_E) << "; "
              << "ICD_CH " << GetValues(clock, 2, ILADataTypes::ICD_CH) << "; "
              << "ICD_BU " << GetValues(clock, 2, ILADataTypes::ICD_BU) << "; "
              << "DR_A " << GetValues(clock, 2, ILADataTypes::DR_A) << "; "
              << "DR_L " << GetValues(clock, 2, ILADataTypes::DR_L) << "; "
              << "DR_S " << GetValues(clock, 2, ILADataTypes::DR_S) << "; "
              << "DR_B " << GetValues(clock, 2, ILADataTypes::DR_B) << "; "
              << "DR_E " << GetValues(clock, 2, ILADataTypes::DR_E) << "; "
              << "DR_CH " << GetValues(clock, 2, ILADataTypes::DR_CH) << "; "
              << "DR_BU " << GetValues(clock, 2, ILADataTypes::DR_BU) << "; "
              << "DR_AR " << GetValues(clock, 2, ILADataTypes::DR_AR) << "; "
              << "DR_AV " << GetValues(clock, 2, ILADataTypes::DR_AV) << "; "
              << "DRC_ARV " << GetValues(clock, 2, ILADataTypes::DRC_ARV)
              << "; "
              << "DRC_ARB " << GetValues(clock, 2, ILADataTypes::DRC_ARB)
              << "; "
              << "DRC_ARA " << GetValues(clock, 2, ILADataTypes::DRC_ARA)
              << "; "
              << "DRC_ARL " << GetValues(clock, 2, ILADataTypes::DRC_ARL)
              << "; "
              << "DRC_RD " << GetValues(clock, 2, ILADataTypes::DRC_RD) << "; "
              << "DRC_RV " << GetValues(clock, 2, ILADataTypes::DRC_RV) << "; "
              << "DRC_RR " << GetValues(clock, 2, ILADataTypes::DRC_RR) << "; "
              << "DRC_RL " << GetValues(clock, 2, ILADataTypes::DRC_RL) << "; "
              << "IB_D " << GetValues(clock, 2, ILADataTypes::IB_D) << "; "
              << "IB_V " << GetValues(clock, 2, ILADataTypes::IB_V) << "; "
              << "IB_S " << GetValues(clock, 2, ILADataTypes::IB_S) << "; "
              << "IB_B " << GetValues(clock, 2, ILADataTypes::IB_B) << "; "
              << "IB_CH " << GetValues(clock, 2, ILADataTypes::IB_CH) << "; "
              << "IB_CL " << GetValues(clock, 2, ILADataTypes::IB_CL) << "; "
              << "IB_L " << GetValues(clock, 2, ILADataTypes::IB_L) << "; "
              << "IB_SI " << GetValues(clock, 2, ILADataTypes::IB_SI) << "; "
              << "IB_VS " << GetValues(clock, 2, ILADataTypes::IB_VS) << "; "
              << "IB_BU " << GetValues(clock, 2, ILADataTypes::IB_BU) << "; "
              << std::endl;
  }
}