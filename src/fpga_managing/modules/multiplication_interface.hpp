#pragma once

#include <array>

namespace dbmstodspi {
namespace fpga_managing {
namespace modules {

/**
 * @brief Interface class which is implemented in the #Multiplication class.
 */
class MultiplicationInterface {
 public:
  virtual ~MultiplicationInterface() = default;

  virtual void DefineActiveStreams(std::array<bool, 16> active_streams) = 0;
  virtual void ChooseMultiplicationResults(
      int chunk_id, std::array<bool, 8> active_positions) = 0;
};

}  // namespace modules
}  // namespace fpga_managing
}  // namespace dbmstodspi