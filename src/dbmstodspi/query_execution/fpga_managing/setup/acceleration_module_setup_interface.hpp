/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once
#include <vector>

#include "accelerated_query_node.hpp"
#include "acceleration_module.hpp"
#include "stream_data_parameters.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Interface for classes which can setup acceleration modules.
 */
class AccelerationModuleSetupInterface {
 public:
  virtual ~AccelerationModuleSetupInterface() = default;
  /**
   * @brief Setup a given module
   * @param acceleration_module Operator object to access initialisation
   * registers.
   * @param module_parameters Parameters to initalise with.
   */
  virtual void SetupModule(AccelerationModule& acceleration_module,
                           const AcceleratedQueryNode& module_parameters) = 0;
  /**
   * @brief Create a module class with memory mapped register addresses.
   * @param memory_manager Memory manager to access registers.
   * @param module_position Position on the FPGA
   * @return Pointer to a module to be initialised.
   */
  virtual auto CreateModule(MemoryManagerInterface* memory_manager,
                            int module_position)
      -> std::unique_ptr<AccelerationModule> = 0;

  /**
   * @brief Method to check if a stream is supposed to be a multi-channel
   * stream.
   * @param is_input_stream Boolean to check input or output streams
   * @param stream_index Index of the stream
   * @return True if the stream is supposed to be multi-channel
   */
  virtual auto IsMultiChannelStream(bool is_input_stream, int stream_index)
      -> bool;

  /**
   * @brief Get multi channel parameters
   * @param is_input Is the stream an input stream.
   * @param stream_index The index of the stream.
   * @param operation_parameters Parameters where multi channel data can be
   * extracted from
   * @return How many channels with how many records there are for this stream.
   */
  virtual auto GetMultiChannelParams(
      bool is_input, int stream_index,
      std::vector<std::vector<int>> operation_parameters)
      -> std::pair<int, int>;

  /**
   * @brief Can the module have any prerequisite modules before it?
   * @return Bool showing if it has the constraint.
   */
  virtual auto IsConstrainedToFirstInPipeline() -> bool;
 protected:
  static auto GetStreamRecordSize(const StreamDataParameters& stream_parameters)
      -> int;
};

}  // namespace orkhestrafs::dbmstodspi