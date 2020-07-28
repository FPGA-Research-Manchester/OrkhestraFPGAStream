#include "DMA.hpp"

#include <cmath>
#include <iostream>

/*
This is the width of the address for the memory mapped register space in every
module. This parameter's value has to match the parameter in the implemented DMA
module.
*/
#define MODULE_ADDRESS_BITS 20

DMA::~DMA() = default;

DMA::DMA(int* volatile ctrl_ax_ibase_address)
    : AccelerationModule(ctrl_ax_ibase_address, 0) {}

// Input Controller
void DMA::setInputControllerParams(int stream_id, int dd_rburst_size,
                                   int records_per_ddr_burst, int buffer_start,
                                   int buffer_end) {
  AccelerationModule::writeToModule(
      stream_id * 4, ((dd_rburst_size - 1) << 24) +
                         (static_cast<int>(log2(records_per_ddr_burst)) << 16) +
                         (buffer_start << 8) + (buffer_end));
}
auto DMA::getInputControllerParams(int stream_id) -> uint32_t {
  return AccelerationModule::readFromModule(stream_id * 4);
}
void DMA::setInputControllerStreamAddress(int stream_id, uintptr_t address) {
  AccelerationModule::writeToModule(((1 << 6) + (stream_id * 4)), address >> 4);
}
auto DMA::getInputControllerStreamAddress(int stream_id) -> uintptr_t {
  return AccelerationModule::readFromModule(((1 << 6) + (stream_id * 4)));
}
void DMA::setInputControllerStreamSize(
    int stream_id, int size) {  // starting size of stream in amount of records
  AccelerationModule::writeToModule(((2 << 6) + (stream_id * 4)), size);
}
auto DMA::getInputControllerStreamSize(int stream_id) -> uint32_t {
  return AccelerationModule::readFromModule(((2 << 6) + (stream_id * 4)));
}
void DMA::startInputController(
    bool stream_active[16]) {  // indicate which streams can be read from DDR
                               // and start processing
  int active_streams = 0;
  for (int i = 15; i >= 0; i--) {
    active_streams = active_streams << 1;
    if (stream_active[i]) {
      active_streams = active_streams + 1;
    }
  }
  AccelerationModule::writeToModule(3 << 6, active_streams);
}
auto DMA::isInputControllerFinished()
    -> bool {  // true if all input streams were read from DDR
  uint32_t active_streams = AccelerationModule::readFromModule(3 << 6);
  return (active_streams == 0);
}
// Input Controller in Crossbar

// How many chunks is a record on a particular streamID
void DMA::setRecordSize(int stream_id, int record_size) {
  AccelerationModule::writeToModule(((1 << 17) + (1 << 8) + (stream_id * 4)),
                                    record_size - 1);
}
// set ChunkID at clock cycle of interfaceCycle for records on a particular
// streamID
void DMA::setRecordChunkIDs(int stream_id, int interface_cycle, int chunk_id) {
  AccelerationModule::writeToModule(
      ((1 << 17) + (1 << 13) + (stream_id << 8) + (interface_cycle << 2)),
      chunk_id);
}

// Output Controller
void DMA::setOutputControllerParams(int stream_id, int dd_rburst_size,
                                    int records_per_ddr_burst, int buffer_start,
                                    int buffer_end) {
  AccelerationModule::writeToModule(
      ((1 << 16) + (stream_id * 4)),
      ((dd_rburst_size - 1) << 24) +
          (static_cast<int>(log2(records_per_ddr_burst)) << 16) +
          (buffer_start << 8) + (buffer_end));
}
auto DMA::getOutputControllerParams(int stream_id) -> uint32_t {
  return AccelerationModule::readFromModule(((1 << 16) + (stream_id * 4)));
}
void DMA::setOutputControllerStreamAddress(int stream_id, uintptr_t address) {
  AccelerationModule::writeToModule(((1 << 16) + (1 << 6) + (stream_id * 4)),
                                    address >> 4);
}
auto DMA::getOutputControllerStreamAddress(int stream_id) -> uintptr_t {
  return AccelerationModule::readFromModule(
      ((1 << 16) + (1 << 6) + stream_id * 4));
}
void DMA::setOutputControllerStreamSize(
    int stream_id, int size) {  // starting size of stream in amount of records
  AccelerationModule::writeToModule(((1 << 16) + (2 << 6) + (stream_id * 4)),
                                    size);
}
auto DMA::getOutputControllerStreamSize(int stream_id)
    -> uint32_t {  // starting size of stream in amount of records
  return AccelerationModule::readFromModule(
      ((1 << 16) + (2 << 6) + (stream_id * 4)));
}
void DMA::startOutputController(
    bool stream_active[16]) {  // indicate which streams can be written to DDR
                               // and start processing
  int active_streams = 0;
  for (int i = 15; i >= 0; i--) {
    active_streams = active_streams << 1;
    if (stream_active[i]) {
      active_streams = active_streams + 1;
    }
  }
  AccelerationModule::writeToModule(((1 << 16) + (3 << 6)), active_streams);
}
auto DMA::isOutputControllerFinished()
    -> bool {  // true if all streams saw EOS from PR modules
  uint32_t active_streams =
      AccelerationModule::readFromModule(((1 << 16) + (3 << 6)));
  return (active_streams == 0);
}

// Input Crossbar from Buffers to Interface
void DMA::setBufferToInterfaceChunk(int stream_id, int clock_cycle, int offset,
                                    int source_chunk4, int source_chunk3,
                                    int source_chunk2, int source_chunk1) {
  /*When 32-bit data packets inside the {clockCycle} clock cycle of a record
  sent to PR Interface of stream with ID {streamID} is read from the BRAM
  buffers in positions  {offset*4}-{offset*4+3}, they can read from any source
  chunk to enable data reordering and duplication. sourceChunk1 represents
  position {offset*4} etc.*/
  AccelerationModule::writeToModule(
      ((2 << 17) + (1 << 16) + (stream_id << 12) + (clock_cycle << 5) +
       (offset << 2)),
      ((source_chunk4 << 24) + (source_chunk3 << 16) + (source_chunk2 << 8) +
       source_chunk1));
}
void DMA::setBufferToInterfaceSourcePosition(int stream_id, int clock_cycle,
                                             int offset, int source_position4,
                                             int source_position3,
                                             int source_position2,
                                             int source_position1) {
  /*When 32-bit data packets inside the {clockCycle} clock cycle of a record
  sent to PR Interface of stream with ID {streamID} is sent to PR at 32-bit data
  positions {offset*4}-{offset*4+3}, they can originate from from any source
  32-bit BRAM to enable data reordering and duplication. sourceChunk1 represents
  position {offset*4} etc..*/
  AccelerationModule::writeToModule(
      ((2 << 17) + (stream_id << 12) + (clock_cycle << 5) + (offset << 2)),
      ((source_position4 << 24) + (source_position3 << 16) +
       (source_position2 << 8) + source_position1));
}

// Input Crossbar from AXI/DDR Input to Buffers
void DMA::setAXItoBufferChunk(int stream_id, int clock_cycle, int offset,
                              int target_chunk4, int target_chunk3,
                              int target_chunk2, int target_chunk1) {
  /*When 32-bit data packets inside the {clockCycle} clock cycle of the AXI read
  data burst of stream with ID {streamID} reach the BRAM buffers in positions
  {offset*4}-{offset*4+3}, they can be written to any target chunk to aid data
  reordering and duplication. sourceChunk1 represents position {offset*4} etc.*/
  AccelerationModule::writeToModule(
      ((2 << 18) + (1 << 17) + (stream_id << 13) + (clock_cycle << 5) +
       (offset << 2)),
      ((target_chunk4 << 24) + (target_chunk3 << 16) + (target_chunk2 << 8) +
       target_chunk1));
}
void DMA::setAXItoBufferSourcePosition(int stream_id, int clock_cycle,
                                       int offset, int source_position4,
                                       int source_position3,
                                       int source_position2,
                                       int source_position1) {
  /*When an AXI read data enters the DMA, the {offset*4}-{offset*4+3} data
  positions for the buffer in the {clockCycle} clock cycle of the AXI
  transaction of stream with ID {streamID}, they can select any 32-bit data
  source from AXI datapath. sourceChunk1 represents position {offset*4} etc..*/
  AccelerationModule::writeToModule(
      ((2 << 18) + (stream_id << 13) + (clock_cycle << 5) + (offset << 2)),
      ((source_position4 << 24) + (source_position3 << 16) +
       (source_position2 << 8) + source_position1));
}

// Output Crossbar from Interface to Buffers
void DMA::setInterfaceToBufferChunk(int stream_id, int clock_cycle, int offset,
                                    int target_chunk4, int target_chunk3,
                                    int target_chunk2, int target_chunk1) {
  /*When 32-bit data packets inside the {clockCycle} clock cycle of a record
  coming from the end of the PR Interface of stream with ID {streamID} is
  written to the BRAM buffers in positions  {offset*4}-{offset*4+3}, they can be
  written to any target chunk to enable data reordering/removal of duplication
  before writing back to DDR. sourceChunk1 represents position {offset*4} etc.*/
  AccelerationModule::writeToModule(
      ((3 << 17) + (1 << 16) + (stream_id << 12) + (clock_cycle << 5) +
       (offset << 2)),
      ((target_chunk4 << 24) + (target_chunk3 << 16) + (target_chunk2 << 8) +
       target_chunk1));
}
void DMA::setInterfaceToBufferSourcePosition(int stream_id, int clock_cycle,
                                             int offset, int source_position4,
                                             int source_position3,
                                             int source_position2,
                                             int source_position1) {
  /*When 32-bit data packets inside the {clockCycle} clock cycle of a record
  coming from PR Interface of stream with ID {streamID} is written to BRAM
  buffers at 32-bit data positions {offset*4}-{offset*4+3}, they can originate
  from from any source 32-bit word from the interface to enable data reordering
  and duplication. sourceChunk1 represents position {offset*4} etc..*/
  AccelerationModule::writeToModule(
      ((3 << 17) + (stream_id << 12) + (clock_cycle << 5) + (offset << 2)),
      ((source_position4 << 24) + (source_position3 << 16) +
       (source_position2 << 8) + source_position1));
}

// Output Crossbar from Buffers to AXI/DDR
void DMA::setBufferToAXIChunk(int stream_id, int clock_cycle, int offset,
                              int source_chunk4, int source_chunk3,
                              int source_chunk2, int source_chunk1) {
  /*32-bit data words are read from the BRAM buffers in positions
  {offset*4}-{offset*4+3} to be used inside the {clockCycle} clock cycle of the
  AXI read data burst of stream with ID {streamID}. Every BRAM can read on a
  different arbitrary position defined by this register.
   sourceChunk1 represents position {offset*4} etc.*/
  AccelerationModule::writeToModule(
      ((3 << 18) + (1 << 17) + (stream_id << 13) + (clock_cycle << 5) +
       (offset << 2)),
      ((source_chunk4 << 24) + (source_chunk3 << 16) + (source_chunk2 << 8) +
       source_chunk1));
}
void DMA::setBufferToAXISourcePosition(int stream_id, int clock_cycle,
                                       int offset, int source_position4,
                                       int source_position3,
                                       int source_position2,
                                       int source_position1) {
  /*Routing information for 32-bit data packets {offset*4}-{offset*4+3} inside
  the {clockCycle} clock cycle of the AXI write data burst of stream with ID
  {streamID}. 32-bit data packets {offset*4}-{offset*4+3} are routed to the DDR
  AXI from any of the BRAM buffers in the middle between the two crossbars.
  sourceChunk1 represents position {offset*4} etc..*/
  AccelerationModule::writeToModule(
      ((3 << 18) + (stream_id << 13) + (clock_cycle << 5) + (offset << 2)),
      ((source_position4 << 24) + (source_position3 << 16) +
       (source_position2 << 8) + source_position1));
}
