#pragma once

namespace dbmstodspi {
namespace fpga_managing {
namespace modules {

/**
 * @brief Interface class for modules which need to get it's result read from the configuration registers.
 */
class ReadBackModuleInterface {
 public:
  virtual ~ReadBackModuleInterface() = default;

  virtual auto ReadResult(int data_position) -> uint32_t = 0;
};

}  // namespace modules
}  // namespace fpga_managing
}  // namespace dbmstodspi