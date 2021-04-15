#pragma once

#include <array>
#include <cstdint>
#include <utility>
#include <bitset>

namespace dbmstodspi {
namespace fpga_managing {
namespace modules {

/**
 * @brief Interface class which is implemented in the #Addition class.
 */
class AdditionInterface {
 public:
  virtual ~AdditionInterface() = default;

  virtual void DefineInput(int stream_id, int chunk_id) = 0;
  virtual void SetInputSigns(std::bitset<8> is_value_negative) = 0;
  virtual void SetLiteralValues(
      std::array<std::pair<uint32_t, uint32_t>, 8> literal_values) = 0;
};

}  // namespace modules
}  // namespace fpga_managing
}  // namespace dbmstodspi