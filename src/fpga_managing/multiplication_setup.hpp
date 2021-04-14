#pragma once
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
   * @brief Method to setup the addition operation acceleration.
   * @param addition_module Instance of the accelerator to have access to the
   * memory mapped configuration space.
   * @param stream_id TODO Make it possible to setup multiple streams and chunks!
   */
  static void SetupMultiplicationModule(modules::MultiplicationInterface& multiplication_module,
                           int active_stream_id);
};

}  // namespace fpga_managing
}  // namespace dbmstodspi