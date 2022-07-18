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
#include <filesystem>
#include <iostream>
#include <numeric>
#include <stdexcept>

#include "logger.hpp"

#ifdef FPGA_AVAILABLE
#include <cstdlib>

#include "fpga.h"
#include "mmio.h"
#include "udma_memory_block.hpp"
#else
#include "virtual_memory_block.hpp"
#endif

using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;
using orkhestrafs::dbmstodspi::MemoryManager;

MemoryManager::~MemoryManager() = default;

void MemoryManager::TestConfigurationTimes(
    std::vector<std::string>& bitstream_name, int repetition_count) {
#ifdef FPGA_AVAILABLE
  const int register_space_size = 10 * 1024 * 1024;
  std::unordered_map<std::string, int> min_times;
  acceleration_instance_ =
      pr_manager_.fpgaLoadStatic("static", register_space_size);
  for (const auto& name : bitstream_name) {
    min_times[name] = std::numeric_limits<int>::max();
  }
  FPGAManager fpga_manager(0);
  for (int i = 0; i < repetition_count; i++) {
    for (const auto& name : bitstream_name) {
      auto begin = std::chrono::steady_clock::now();
      fpga_manager.loadPartial(name);
      auto end = std::chrono::steady_clock::now();
      auto time =
          std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
              .count();
      if (time < min_times[name]) {
        min_times[name] = time;
      }
    }
  }

  for (const auto& [name, time] : min_times) {
    std::cout << name << "CONFIGURATION:" << std::to_string(time) << std::endl;
  }

  acceleration_instance_ =
      pr_manager_.fpgaLoadStatic("static", register_space_size);
#endif
}

void MemoryManager::LoadStatic() {
  // Addressable register space on the FPGA. No need to preserve space for the
  // whole addressable space, for the supported library 10 MB is enough.
  const int register_space_size = 10 * 1024 * 1024;
#ifdef FPGA_AVAILABLE
  Log(LogLevel::kDebug, "Loading static");

  std::string static_bitstream = "static";

  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();

  acceleration_instance_ =
      pr_manager_.fpgaLoadStatic(static_bitstream, register_space_size);
  /*acceleration_instance_ =
      pr_manager_.fpgaLoadStatic("DSPI_filtering_linear_sort",
     register_space_size);*/

  register_memory_block_ = acceleration_instance_.prmanager->accelRegs;

  SetFPGATo300MHz();
  // SetFPGATo100MHz();

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "STATIC CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
  Log(LogLevel::kInfo,
      "Extra config time = " +
          std::to_string(
              std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                  .count()) +
          "[ms]");
  Log(LogLevel::kDebug, "Static loaded: " + static_bitstream);
#else
  /*throw std::runtime_error("Can't load anything!");*/
  register_space_ = std::vector<uint32_t>(register_space_size, -1);
#endif
  loaded_register_space_size_ = register_space_size;
  loaded_bitstream_ = "static";
}

// This is a debug method with the FPGA available.
void MemoryManager::MeasureConfigurationSpeed(
    const std::set<std::string>& bitstreams_to_measure) {
#ifdef FPGA_AVAILABLE
  std::unordered_map<std::string, std::vector<int>> configuration_times;
  for (const auto& bitstream : bitstreams_to_measure) {
    configuration_times.insert({bitstream, {}});
  }
  FPGAManager fpga_manager(0);
  const int repetition_count = 500;
  for (int i = 0; i < repetition_count; i++) {
    for (const auto& bitstream : bitstreams_to_measure) {
      std::chrono::steady_clock::time_point begin =
          std::chrono::steady_clock::now();
      fpga_manager.loadPartial(bitstream);
      std::chrono::steady_clock::time_point end =
          std::chrono::steady_clock::now();
      configuration_times.at(bitstream).push_back(
          std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
              .count());
    }
  }
  for (const auto& [bitstream_name, bitstream_config_times] :
       configuration_times) {
    auto min_config_time_microseconds = *min_element(
        bitstream_config_times.begin(), bitstream_config_times.end());
    std::filesystem::path p{bitstream_name};
    auto size_bytes = std::filesystem::file_size(p);
    // Just to show we are reporting MB/s
    auto configuration_speed =
        (static_cast<double>(size_bytes) / 1000000.0) /
        (static_cast<double>(min_config_time_microseconds) / 1000000.0);
    std::cout << bitstream_name << " MIN CONFIGURATION: "
              << std::to_string(min_config_time_microseconds) << std::endl;

    std::cout << bitstream_name
              << " SIZE IN BYTES: " << std::to_string(size_bytes) << std::endl;

    std::cout << bitstream_name
              << " CONFIG SPEED MB/s: " << std::to_string(configuration_speed)
              << std::endl;
  }
  std::exit(0);

#else
  throw std::runtime_error(
      "We can measure bitstream configuration times only with an FPGA!");
#endif
}

void MemoryManager::LoadPartialBitstream(
    const std::vector<std::string>& bitstream_name,
    DMAInterface& dma_engine) {
  if (loaded_bitstream_ != "static") {
    throw std::runtime_error("Can't load partial bitstreams without static!");
  }

#ifdef FPGA_AVAILABLE
  dma_engine.DecoupleFromPRRegion();
  FPGAManager fpga_manager(0);
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
  /*std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
  fpga_manager.loadPartial("l1024.bin");
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "L1024 PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("m128.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "m128 PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;

  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("pr_region.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "pr_region PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;

  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("m32.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "m32 PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;

  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("sum.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "SUM PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;

  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("add.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "ADD PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;

  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("mul.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "MUL PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;

  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("v2binPartial_LinearSort512_7_36.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "L512 PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("b_col.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "b_col PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("d_col.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "d_col PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("m_col.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "m_col PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("join.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "join PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("m64.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "m64 PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
  begin = std::chrono::steady_clock::now();
  fpga_manager.loadPartial("filter.bin");
  end = std::chrono::steady_clock::now();
  std::cout << "filter PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;*/

  for (const auto& name : bitstream_name) {
    Log(LogLevel::kDebug, "Loading PR bitstream:" + name);
    fpga_manager.loadPartial(name);
  }
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "PR CONFIGURATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
#else
  // Don't do anything
  /*throw std::runtime_error("Can't load anything!");*/
#endif
}

void MemoryManager::LoadBitstreamIfNew(const std::string& bitstream_name,
                                       const int register_space_size) {
  if (bitstream_name != loaded_bitstream_ ||
      register_space_size != loaded_register_space_size_) {
#ifdef FPGA_AVAILABLE
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

auto MemoryManager::GetAvailableMemoryBlock() -> MemoryBlockInterface* {
  if (available_memory_blocks_.empty()) {
    return AllocateMemoryBlock();
  }
  MemoryBlockInterface* available_memory_block = available_memory_blocks_.top();
  available_memory_blocks_.pop();
  return available_memory_block;
}

auto MemoryManager::GetVirtualRegisterAddress(int offset)
    -> volatile uint32_t* {
  if (!loaded_register_space_size_) {
    throw std::runtime_error("No bitstream loaded!");
  }
#ifdef FPGA_AVAILABLE
  return &(register_memory_block_[offset / 4]);
#else
  return &(register_space_[offset]);
#endif
}

auto MemoryManager::AllocateMemoryBlock() -> MemoryBlockInterface* {
  if (memory_block_count_++ >= kMaxPossibleAllocations) {
    throw std::runtime_error("Can't allocate any more memory!");
  }
#ifdef FPGA_AVAILABLE
  current_memory_blocks_.push_back(std::move(std::make_unique<UDMAMemoryBlock>(
      udma_repo_.device(memory_block_count_ - 1))));
#else
  current_memory_blocks_.push_back(
      std::move(std::make_unique<VirtualMemoryBlock>()));
#endif
  return current_memory_blocks_.back().get();
}

void MemoryManager::FreeMemoryBlock(
    MemoryBlockInterface* memory_block_pointer) {
  available_memory_blocks_.push(memory_block_pointer);
}

void MemoryManager::SetFPGATo300MHz() { SetFPGAClockSpeed(0x10500); }
void MemoryManager::SetFPGATo100MHz() { SetFPGAClockSpeed(0x10F00); }

void MemoryManager::SetFPGAClockSpeed(int speed_value) {
#ifdef FPGA_AVAILABLE
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

void MemoryManager::UnSetPCAP() {
#ifdef FPGA_AVAILABLE
  auto pcap_memory_map = mmioGetMmap("/dev/mem", 0xFFCA3000, 256);
  if (pcap_memory_map.fd == -1)
    throw std::runtime_error("Failed to mmap clock area");
  auto pcap_area_pointer = (uint32_t*)pcap_memory_map.mmap;

  volatile uint32_t* pcap_ptr = &(pcap_area_pointer[8 / 4]);
  uint32_t pcap_value = *pcap_ptr;
  pcap_value &= ~(1 << 0);
  *pcap_ptr = pcap_value;

  pcap_value = *pcap_ptr;
  auto mask_value = 0x00000001;
  pcap_value = mask_value & pcap_value;

  if ((pcap_value != 0x00000000)) {
    throw std::runtime_error(
        "PCAP_CTRL[0] was not cleared.  Bit[0] should be 0: " +
        std::to_string(pcap_value));
  } else {
    Log(LogLevel::kDebug, "PCAP is disabled, ICAP is enabled");
  }
#else
  throw std::runtime_error("NoFPGAConnected!");
#endif
}