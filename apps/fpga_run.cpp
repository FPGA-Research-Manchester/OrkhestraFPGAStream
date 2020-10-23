#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "data_manager.hpp"
#include "fpga_manager.hpp"
#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

#include "cynq.h"
#include "udma.h"

#include "unistd.h"

/*
Filter: (price < 12000)
1000 rows
 Column |         Type          |
--------+-----------------------+
 id     | bigint                |
 make   | character varying(32) |
 model  | character varying(32) |
 price  | integer               |
*/

auto manual_aligned_alloc(uint8_t alignment, uint64_t size) -> void* {
  auto potentially_unaligned_buffer = (uint64_t)malloc(size + alignment);
  uint64_t aligned_buffer =
      ((potentially_unaligned_buffer + alignment) / alignment) * alignment;
  auto* buffer = (uint8_t*)aligned_buffer;
  *(buffer - 1) = aligned_buffer - potentially_unaligned_buffer;
  return buffer;
}

void manual_aligned_free(void* ptr) {
  if (ptr) {
    auto* buffer = static_cast<uint8_t*>(ptr);
    uint8_t shift = *(buffer - 1);
    buffer -= shift;
    free(buffer);
  }
}

auto main() -> int {
  std::cout << "Starting main" << std::endl;
  std::vector<int> data_type_sizes;
  data_type_sizes.push_back(1);
  data_type_sizes.push_back(8);
  data_type_sizes.push_back(8);
  data_type_sizes.push_back(1);

  int record_size = 0;
  for (int row_size : data_type_sizes) {
    record_size += row_size;
  }

  std::vector<std::vector<std::string>> db_data;
  DataManager::AddStringDataFromCSV("MOCK_DATA.csv", db_data);

  std::vector<uint32_t> input_memory_area;
  DataManager::AddIntegerDataFromStringData(db_data, input_memory_area);
  db_data.clear();

  int record_count = input_memory_area.size() / record_size;
  int output_memory_size =
      (record_count +
       record_count %
           StreamParameterCalculator::FindMinViableRecordsPerDDRBurst(
               query_acceleration_constants::kDdrBurstSize, record_size)) *
      record_size;

  UdmaRepo repo;
  UdmaDevice* input_device = repo.device(0);
  UdmaDevice* output_device = repo.device(1);

  volatile uint32_t* input = (uint32_t*)input_device->map();
  volatile uint32_t* output = (uint32_t*)output_device->map();

  for (int i = 0; i < input_memory_area.size(); i++) {
    input[i] = input_memory_area[i];
  }

  volatile uint32_t* input_data_phy =
      reinterpret_cast<volatile uint32_t*>(input_device->phys_addr);
  volatile uint32_t* output_data_phy =
      reinterpret_cast<volatile uint32_t*>(output_device->phys_addr);

  PRManager prmanager;
  StaticAccelInst db_accel = prmanager.fpgaLoadStatic("DSPI_filtering");

  std::cout << "Main initialisation done!" << std::endl;
  FPGAManager fpga_manager(&db_accel);
  fpga_manager.SetupQueryAcceleration(input_data_phy, output_data_phy,
                                      record_size,
                                      record_count);
  std::vector<int> result_sizes = fpga_manager.RunQueryAcceleration();
  std::cout << "Query done!" << std::endl;
  DataManager::AddStringDataFromIntegerData(
      std::vector<uint32_t>(output, output + (result_sizes[0] * record_size)),
      db_data, data_type_sizes);
  DataManager::PrintStringData(db_data);

  sleep(2);

  return 0;
}
