#include "ila.hpp"

#include <iostream>

void ILA::startILAs() {
  ILA::WriteToModule(0x10000000, 1);
  ILA::WriteToModule(0x10000004, 3);
  ILA::WriteToModule(0x12000000, 1);
  ILA::WriteToModule(0x12000004, 3);
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
  }
  else if (ila_id == 1) {
    return (0x12000000 + (clock << 11) + (offset << 2));
  }
  else if (ila_id == 2) {
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
              << getValues(clock, ila_id, ILADataTypes::kClockCycle)
              << std::endl
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
        << getValues(clock, 2, ILADataTypes::kAxiClockCycle) << std::endl
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