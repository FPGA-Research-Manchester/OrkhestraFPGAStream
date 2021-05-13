#pragma once

#include <bitset>

namespace dbmstodspi::fpga_managing::modules {

/**
 * @brief Interface class which is implemented in the #Multiplication class.
 */
class MultiplicationInterface {
 public:
  virtual ~MultiplicationInterface() = default;

  virtual void DefineActiveStreams(std::bitset<16> active_streams) = 0;
  virtual void ChooseMultiplicationResults(int chunk_id,
                                           std::bitset<8> active_positions) = 0;
};

}  // namespace dbmstodspi::fpga_managing::modules