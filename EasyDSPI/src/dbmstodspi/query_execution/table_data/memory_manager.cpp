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

#include "memory_manager.hpp"

#include <chrono>
#include <iostream>
#include <stdexcept>

#include "logger.hpp"

#ifdef _FPGA_AVAILABLE
#include "mmio.h"
#include "udma_memory_block.hpp"
#else
#include "virtual_memory_block.hpp"
#endif

using easydspi::dbmstodspi::Log;
using easydspi::dbmstodspi::LogLevel;
using easydspi::dbmstodspi::MemoryManager;

MemoryManager::~MemoryManager() = default;

void MemoryManager::LoadBitstreamIfNew(const std::string& bitstream_name,
                                       const int register_space_size) {
  if (bitstream_name != loaded_bitstream_ ||
      register_space_size != loaded_register_space_size_) {
#ifdef _FPGA_AVAILABLE
    Log(LogLevel::kDebug, "Loading " + bitstream_name);

    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();

    acceleration_instance_ =
        pr_manager_.fpgaLoadStatic(bitstream_name, register_space_size);

    std::chrono::steady_clock::time_point end =
        std::chrono::steady_clock::now();
    register_memory_block_ = acceleration_instance_.prmanager->accelRegs;
    SetFPGATo300MHz();
    end = std::chrono::steady_clock::now();
    Log(LogLevel::kInfo,
        "Extra config time = " +
            std::to_string(
                std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                      begin)
                    .count()) +
            "[ms]");
    Log(LogLevel::kDebug, bitstream_name + " loaded!");
#else
    register_space_ = std::vector<uint32_t>(register_space_size, -1);
#endif
    loaded_register_space_size_ = register_space_size;
    loaded_bitstream_ = bitstream_name;
  }
}

auto MemoryManager::GetAvailableMemoryBlock()
    -> std::unique_ptr<MemoryBlockInterface> {
  if (available_memory_blocks_.empty()) {
    return AllocateMemoryBlock();
  }
  std::unique_ptr<MemoryBlockInterface> available_memory_block =
      std::move(available_memory_blocks_.top());
  available_memory_blocks_.pop();
  return available_memory_block;
}

auto MemoryManager::GetVirtualRegisterAddress(int offset)
    -> volatile uint32_t* {
  if (!loaded_register_space_size_) {
    throw std::runtime_error("No bitstream loaded!");
  }
#ifdef _FPGA_AVAILABLE
  return &(register_memory_block_[offset / 4]);
#else
  return &(register_space_[offset]);
#endif
}

auto MemoryManager::AllocateMemoryBlock()
    -> std::unique_ptr<MemoryBlockInterface> {
  if (memory_block_count_++ >= kMaxPossibleAllocations) {
    throw std::runtime_error("Can't allocate any more memory!");
  }
#ifdef _FPGA_AVAILABLE
  return std::make_unique<UDMAMemoryBlock>(
      udma_repo_.device(memory_block_count_ - 1));
#else
  return std::make_unique<VirtualMemoryBlock>();
#endif
}

void MemoryManager::FreeMemoryBlock(
    std::unique_ptr<MemoryBlockInterface> memory_block_pointer) {
  available_memory_blocks_.push(std::move(memory_block_pointer));
}

void MemoryManager::SetFPGATo300MHz() { SetFPGAClockSpeed(0x10500); }
void MemoryManager::SetFPGATo100MHz() { SetFPGAClockSpeed(0x10F00); }

void MemoryManager::SetFPGAClockSpeed(int speed_value) {
#ifdef _FPGA_AVAILABLE
  auto clock_memory_map = mmioGetMmap("/dev/mem", 0xFF5E0000, 1024);
  if (clock_memory_map.fd == -1)
    throw std::runtime_error("Failed to mmap clock area");
  auto clock_area_pointer = (uint32_t*)clock_memory_map.mmap;

  volatile uint32_t* pl_clk0 = &(clock_area_pointer[192 / 4]);
  uint32_t value = *pl_clk0;
  value = value & 0xFEFFFFFF;
  *pl_clk0 = value;
  value = value & 0xFFC0C0FF;
  value = value | speed_value;
  *pl_clk0 = value;
  value = value | 0x01000000;
  *pl_clk0 = value;
#else
  throw std::runtime_error("NoFPGAConnected!");
#endif
}