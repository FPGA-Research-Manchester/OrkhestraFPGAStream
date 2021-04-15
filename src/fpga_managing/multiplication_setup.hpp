#pragma once
#include <vector>

#include "multiplication_interface.hpp"

namespace dbmstodspi {
namespace fpga_managing {

/**
 * @brief Class to calculate the Multiplication module setup.
 */
class MultiplicationSetup {
 private:
 public:
  /**
   * @brief Method to setup the multiplication operation acceleration.
   *
   * The operation parameters will consist of multiple vectors where the first
   * integer notes which chunk the following selection data is configuring.
   *
   * @param addition_module Instance of the accelerator to have access to the
   * memory mapped configuration space.
   * @param active_stream_ids Which streams this module should operate on?
   * @param operation_parameters The operation parameters to configure this
   * module.
   */
  static void SetupMultiplicationModule(
      modules::MultiplicationInterface& multiplication_module,
      std::vector<int> active_stream_ids,
      const std::vector<std::vector<int>>& operation_parameters);
};

}  // namespace fpga_managing
}  // namespace dbmstodspi