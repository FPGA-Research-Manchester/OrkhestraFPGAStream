#pragma once

#include <cstdint>

namespace dbmstodspi::fpga_managing::modules {

/**
 * @brief Interface class for modules which need to get it's result read from
 * the configuration registers.
 */
class ReadBackModuleInterface {
 public:
  virtual ~ReadBackModuleInterface() = default;

  virtual auto ReadResult(int data_position) -> uint32_t = 0;
};

}  // namespace dbmstodspi::fpga_managing::modules