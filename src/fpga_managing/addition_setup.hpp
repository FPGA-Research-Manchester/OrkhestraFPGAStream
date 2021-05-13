#pragma once
#include <vector>
#include <bitset>

#include "addition_interface.hpp"

namespace dbmstodspi::fpga_managing {

/**
 * @brief Class to calculate the Addition module setup.
 */
class AdditionSetup {
 private:
  static auto ReverseLiteralValues(std::vector<int> input_constant_values)
      -> std::array<std::pair<uint32_t, uint32_t>, 8>;
  static auto ReverseNegationSpecification(
      std::vector<int> negation_specification) -> std::bitset<8>;
 public:
  /**
   * @brief Method to setup the addition operation acceleration.
   *
   * Operation parameters has to consist of 3 vectors:
   * 1) For chunk ID
   * 2) Which input value should be negated
   * 3) What are the constant values used
   *
   * @param addition_module Instance of the accelerator to have access to the
   * memory mapped configuration space.
   * @param stream_id Which stream is going to have constants added.
   * @param operation_parameters Vector of operation parameters to setup the
   * module with
   */
  static void SetupAdditionModule(
      modules::AdditionInterface& addition_module, int stream_id,
      const std::vector<std::vector<int>>& operation_parameters);
};

}  // namespace dbmstodspi::fpga_managing