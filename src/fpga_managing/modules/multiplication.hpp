#pragma once
#include "acceleration_module.hpp"
#include "multiplication_interface.hpp"
#include "memory_manager_interface.hpp"

namespace dbmstodspi {
namespace fpga_managing {
namespace modules {

/**
 * @brief Class which implements low level memory writes to the multiplication operation accelerator.
 */
class Multiplication : public AccelerationModule,
                       public MultiplicationInterface {
 private:
 public:
  ~Multiplication() override = default;
  /**
   * @brief Constructor to set the memory manager instance to access memory
   * mapped registers.
   * @param memory_manager Memory manager instance to access memory mapping.
   * @param module_position Position of the module in the bitstream.
   */
  explicit Multiplication(MemoryManagerInterface* memory_manager,
                          int module_position)
      : AccelerationModule(memory_manager, module_position){};

  /**
   * @brief Defube which streams need to have the multiplication operation.
   * @param active_streams Array of booleans noting active streams.
  */
  void DefineActiveStreams(std::array<bool, 16> active_streams) override;
  /**
   * @brief Define which positions in which chunk should have the multiplication results.
   * @param chunk_id Which chunk is being configured.
   * @param active_positions Which positions should have the multiplication result.
  */
  void ChooseMultiplicationResults(int chunk_id, std::array<bool, 8> active_positions) override;
};

}  // namespace modules
}  // namespace fpga_managing
}  // namespace dbmstodspi