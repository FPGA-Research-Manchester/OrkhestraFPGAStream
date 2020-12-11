#include "ila.hpp"

#include <iostream>

void ILA::startILAs() {
  ILA::WriteToModule(0x10000000, 1);
  ILA::WriteToModule(0x10000004, 3);
  ILA::WriteToModule(0x12000000, 1);
  ILA::WriteToModule(0x12000004, 3);
  ILA::WriteToModule(0x14000000, 1);
}

void ILA::startAxiILA() { ILA::WriteToModule(0x14000000, 1); }

auto ILA::getValues(int clock_cycle, int location, ILADataTypes data_type)
    -> uint32_t {
  return ILA::ReadFromModule(
      ILA::calcAddress(clock_cycle, location, static_cast<int>(data_type)));
}

auto ILA::calcAddress(int clock, int ila_id, int offset) -> int {
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
              << getValues(clock, ila_id, ILADataTypes::kClockCycle) << "; "
              << "TYPE " << getValues(clock, ila_id, ILADataTypes::kType)
              << "; "
              << "STREAMID "
              << getValues(clock, ila_id, ILADataTypes::kStreamID) << "; "
              << "CHUNKID " << getValues(clock, ila_id, ILADataTypes::kChunkID)
              << "; "
              << "STATE " << getValues(clock, ila_id, ILADataTypes::kState)
              << "; "
              << "CHANNELID "
              << getValues(clock, ila_id, ILADataTypes::kChannelID) << "; "
              << "LAST " << getValues(clock, ila_id, ILADataTypes::kLast)
              << "; "
              << "DATA_15 "
              << getValues(clock, ila_id, ILADataTypes::kDataAtPos15) << "; "
              << "INSTR_CHANNELID "
              << getValues(clock, ila_id, ILADataTypes::kInstrChannelID) << "; "
              << "INSTR_PARAM "
              << getValues(clock, ila_id, ILADataTypes::kInstrParam) << "; "
              << "INSTR_STREAMID "
              << getValues(clock, ila_id, ILADataTypes::kInstrStreamID) << "; "
              << "INSTR_TYPE "
              << getValues(clock, ila_id, ILADataTypes::kInstrType) << "; "
              << "JOIN_STATE "
              << getValues(clock, ila_id, ILADataTypes::kJoinState) << "; "
              << std::endl;
  }
}

void ILA::PrintAxiILAData(int max_clock) {
  for (int clock = 0; clock < max_clock; clock++) {
    std::cout
        << "ILA " << 2 << " CLOCK " << clock << ":"
        << "kAxiClockCycle "
        << getValues(clock, 2, ILADataTypes::kAxiClockCycle) << "; "
        << "kAxiAWV " << getValues(clock, 2, ILADataTypes::kAxiAWV) << "; "
        << "kAxiAWR " << getValues(clock, 2, ILADataTypes::kAxiAWR) << "; "
        << "kAxiAWA " << getValues(clock, 2, ILADataTypes::kAxiAWA) << "; "
        << "kAxiWV " << getValues(clock, 2, ILADataTypes::kAxiWV) << "; "
        << "kAxiWR " << getValues(clock, 2, ILADataTypes::kAxiWR) << "; "
        << "kAxiWD " << getValues(clock, 2, ILADataTypes::kAxiWD) << "; "
        << "kAxiWS " << getValues(clock, 2, ILADataTypes::kAxiWS) << "; "
        << "kAxiBV " << getValues(clock, 2, ILADataTypes::kAxiBV) << "; "
        << "kAxiBR " << getValues(clock, 2, ILADataTypes::kAxiBR) << "; "
        << "kAxiBRESP " << getValues(clock, 2, ILADataTypes::kAxiBRESP) << "; "
        << "kAxiARV " << getValues(clock, 2, ILADataTypes::kAxiARV) << "; "
        << "kAxiARR " << getValues(clock, 2, ILADataTypes::kAxiARR) << "; "
        << "kAxiARA " << getValues(clock, 2, ILADataTypes::kAxiARA) << "; "
        << "kAxiRV " << getValues(clock, 2, ILADataTypes::kAxiRV) << "; "
        << "kAxiRR " << getValues(clock, 2, ILADataTypes::kAxiRR) << "; "
        << "kAxiRD " << getValues(clock, 2, ILADataTypes::kAxiRD) << "; "
        << "kAxiRRESP " << getValues(clock, 2, ILADataTypes::kAxiRRESP) << "; "
        << std::endl;
  }
}

void ILA::PrintDMAILAData(int max_clock) {
  for (int clock = 0; clock < max_clock; clock++) {
    std::cout << std::hex << "ILA " << 2 << " CLOCK " << clock << ":"
              << "IC_S " << getValues(clock, 2, ILADataTypes::IC_S) << "; "
              << "IC_CH " << getValues(clock, 2, ILADataTypes::IC_CH) << "; "
              << "IC_I " << getValues(clock, 2, ILADataTypes::IC_I) << "; "
              << "IC_IP " << getValues(clock, 2, ILADataTypes::IC_IP) << "; "
              << "IC_V " << getValues(clock, 2, ILADataTypes::IC_V) << "; "
              << "IC_P " << getValues(clock, 2, ILADataTypes::IC_P) << "; "
              << "ICD_BUSY " << getValues(clock, 2, ILADataTypes::ICD_BUSY)
              << "; "
              << "ICD_S " << getValues(clock, 2, ILADataTypes::ICD_S) << "; "
              << "ICD_B " << getValues(clock, 2, ILADataTypes::ICD_B) << "; "
              << "ICD_E " << getValues(clock, 2, ILADataTypes::ICD_E) << "; "
              << "ICD_CH " << getValues(clock, 2, ILADataTypes::ICD_CH) << "; "
              << "ICD_BU " << getValues(clock, 2, ILADataTypes::ICD_BU) << "; "
              << "DR_A " << getValues(clock, 2, ILADataTypes::DR_A) << "; "
              << "DR_L " << getValues(clock, 2, ILADataTypes::DR_L) << "; "
              << "DR_S " << getValues(clock, 2, ILADataTypes::DR_S) << "; "
              << "DR_B " << getValues(clock, 2, ILADataTypes::DR_B) << "; "
              << "DR_E " << getValues(clock, 2, ILADataTypes::DR_E) << "; "
              << "DR_CH " << getValues(clock, 2, ILADataTypes::DR_CH) << "; "
              << "DR_BU " << getValues(clock, 2, ILADataTypes::DR_BU) << "; "
              << "DR_AR " << getValues(clock, 2, ILADataTypes::DR_AR) << "; "
              << "DR_AV " << getValues(clock, 2, ILADataTypes::DR_AV) << "; "
              << "DRC_ARV " << getValues(clock, 2, ILADataTypes::DRC_ARV)
              << "; "
              << "DRC_ARB " << getValues(clock, 2, ILADataTypes::DRC_ARB)
              << "; "
              << "DRC_ARA " << getValues(clock, 2, ILADataTypes::DRC_ARA)
              << "; "
              << "DRC_ARL " << getValues(clock, 2, ILADataTypes::DRC_ARL)
              << "; "
              << "DRC_RD " << getValues(clock, 2, ILADataTypes::DRC_RD) << "; "
              << "DRC_RV " << getValues(clock, 2, ILADataTypes::DRC_RV) << "; "
              << "DRC_RR " << getValues(clock, 2, ILADataTypes::DRC_RR) << "; "
              << "DRC_RL " << getValues(clock, 2, ILADataTypes::DRC_RL) << "; "
              << "IB_D " << getValues(clock, 2, ILADataTypes::IB_D) << "; "
              << "IB_V " << getValues(clock, 2, ILADataTypes::IB_V) << "; "
              << "IB_S " << getValues(clock, 2, ILADataTypes::IB_S) << "; "
              << "IB_B " << getValues(clock, 2, ILADataTypes::IB_B) << "; "
              << "IB_CH " << getValues(clock, 2, ILADataTypes::IB_CH) << "; "
              << "IB_CL " << getValues(clock, 2, ILADataTypes::IB_CL) << "; "
              << "IB_L " << getValues(clock, 2, ILADataTypes::IB_L) << "; "
              << "IB_SI " << getValues(clock, 2, ILADataTypes::IB_SI) << "; "
              << "IB_VS " << getValues(clock, 2, ILADataTypes::IB_VS) << "; "
              << "IB_BU " << getValues(clock, 2, ILADataTypes::IB_BU) << "; "
              << std::endl;
  }
}