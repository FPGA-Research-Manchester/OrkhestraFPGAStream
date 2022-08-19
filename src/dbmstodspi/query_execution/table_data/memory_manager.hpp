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

#pragma once
#include <cstdint>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "memory_block_interface.hpp"
#include "memory_manager_interface.hpp"
#ifdef FPGA_AVAILABLE
#include "cynq.h"
#include "udma.h"
#endif

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Memory manager class to setup the FPGA memory accesses and to
 * facilitate the memory mapping.
 */
class MemoryManager : public MemoryManagerInterface {
 private:
  long latest_config_time_ = 0;
  std::vector<std::unique_ptr<MemoryBlockInterface>> current_memory_blocks_;
  std::stack<MemoryBlockInterface*> available_memory_blocks_;
  int memory_block_count_ = 0;
  // Set in
  // https://github.com/FPGA-Research-Manchester/fos/blob/fdac37e188e217293d296d9973c22500c8a4367c/udmalib/setupUdma.sh#L4
  static const int kMaxPossibleAllocations =
      8;  // The script is changed to allocate 6 X 280MB memory blocks.

  std::string loaded_bitstream_;
  int loaded_register_space_size_ = 0;
#ifdef FPGA_AVAILABLE
  uint32_t* register_memory_block_;
  UdmaRepo udma_repo_;
  // Store to not delete the instances
  PRManager pr_manager_;
  StaticAccelInst acceleration_instance_;
#else
  std::vector<uint32_t> register_space_;
#endif
 public:
  auto GetTime() -> long override;
  void MeasureConfigurationSpeed(
      const std::set<std::string>& bitstreams_to_measure) override;

  ~MemoryManager() override;

  /**
   * @brief Load a new bitstream if it isn't loaded already.
   * @param bitstream_name Bitstream file path to be loaded.
   * @param register_space_size How much register memory space is required.
   */
  void LoadBitstreamIfNew(const std::string& bitstream_name,
                          int register_space_size) override;

  /**
   * @brief Get virtual register address given an offset from the base address.
   * @param offset Memory offset range from the base address.
   * @return Pointer to the required virtual memory.
   */
  auto GetVirtualRegisterAddress(int offset) -> volatile uint32_t* override;
  /**
   * @brief Get a pointer to a memory block which isn't used.
   * @return Pointer to a DDR memory block.
   */
  auto GetAvailableMemoryBlock() -> MemoryBlockInterface* override;
  /**
   * @brief Mark a memory block as free.
   * @param memory_block_pointer Pointer to the unused DDR memory block.
   */
  void FreeMemoryBlock(MemoryBlockInterface* memory_block_pointer) override;

  // Quick methods to do PR loading.
  void LoadStatic(int clock_speed) override;
  void LoadPartialBitstream(const std::vector<std::string>& bitstream_name,
                            DMAInterface& dma_engine) override;

 private:
  auto AllocateMemoryBlock() -> MemoryBlockInterface* override;
  static void SetFPGAClockSpeed(int speed_value);
  static void SetFPGATo300MHz();
  static void SetFPGATo100MHz();
  static void UnSetPCAP();
  void TestConfigurationTimes(std::vector<std::string>& bitstream_name,
                              int repetition_count);
  std::vector<std::string> all_bitstreams_ = {
      "TAA_2.bin",
      "TAA_5.bin",
      "TAA_8.bin",
      "TAA_11.bin",
      "TAA_14.bin",
      "TAA_17.bin",
      "TAA_20.bin",
      "TAA_23.bin",
      "TAA_26.bin",
      "TAA_29.bin",
      "TAA_32.bin",
      "TAA_35.bin",
      "TAA_38.bin",
      "TAA_41.bin",
      "TAA_44.bin",
      "TAA_47.bin",
      "TAA_50.bin",
      "TAA_53.bin",
      "TAA_56.bin",
      "TAA_59.bin",
      "TAA_62.bin",
      "TAA_65.bin",
      "TAA_68.bin",
      "TAA_71.bin",
      "TAA_74.bin",
      "TAA_77.bin",
      "TAA_80.bin",
      "TAA_83.bin",
      "TAA_86.bin",
      "TAA_89.bin",
      "TAA_92.bin",
      "TAA_95.bin",
      "RT_2.bin",
      "RT_5.bin",
      "RT_8.bin",
      "RT_11.bin",
      "RT_14.bin",
      "RT_17.bin",
      "RT_20.bin",
      "RT_23.bin",
      "RT_26.bin",
      "RT_29.bin",
      "RT_32.bin",
      "RT_35.bin",
      "RT_38.bin",
      "RT_41.bin",
      "RT_44.bin",
      "RT_47.bin",
      "RT_50.bin",
      "RT_53.bin",
      "RT_56.bin",
      "RT_59.bin",
      "RT_62.bin",
      "RT_65.bin",
      "RT_68.bin",
      "RT_71.bin",
      "RT_74.bin",
      "RT_77.bin",
      "RT_80.bin",
      "RT_83.bin",
      "RT_86.bin",
      "RT_89.bin",
      "RT_92.bin",
      "RT_95.bin",
      "binPartial_Filter_7_36.bin",
      "binPartial_Filter_37_66.bin",
      "binPartial_Filter_67_96.bin",
      "binPartial_LinearSort512_7_36",
      "binPartial_LinearSort512_37_66.bin",
      "binPartial_LinearSort512_67_96.bin",
      "binPartial_MergeJoin2K_7_36.bin",
      "binPartial_MergeJoin2K_37_66.bin",
      "binPartial_MergeJoin2K_67_96.bin",
      "binPartial_MergeSort64_7_36.bin",
      "binPartial_MergeSort64_37_66.bin",
      "binPartial_MergeSort64_67_96.bin",
      "binPartial_AggregateGlobalSum_55_63.bin",
      "binPartial_ConstArith64b_85_96.bin",
      "binPartial_DecMult64b_64_84.bin",
      "binPartial_DecMult64b_34_54.bin",
      "binPartial_DecMult64b_4_24.bin",
      "binPartial_ConstArith64b_55_66.bin",
      "binPartial_ConstArith64b_25_36.bin",
      "binPartial_AggregateGlobalSum_85_93.bin",
      "binPartial_AggregateGlobalSum_25_33.bin"};
};

}  // namespace orkhestrafs::dbmstodspi
