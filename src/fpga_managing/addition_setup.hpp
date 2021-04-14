#pragma once
#include "addition_interface.hpp"

namespace dbmstodspi {
namespace fpga_managing {

/**
 * @brief Class to calculate the Addition module setup.
 */
class AdditionSetup {
 private:
  // TODO: Make it possible to give any normal double and then convert it into 2
  // 32 bit values in this class.
 public:
  /**
   * @brief Method to setup the addition operation acceleration.
   * @param addition_module Instance of the accelerator to have access to the
   * memory mapped configuration space.
   * @param stream_id Which stream is going to have constants added.
   */
  static void SetupAdditionModule(modules::AdditionInterface& addition_module,
                           int stream_id);
};

}  // namespace fpga_managing
}  // namespace dbmstodspi