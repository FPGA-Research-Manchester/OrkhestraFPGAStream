#pragma once
#include <cstdint>

#include "acceleration_module.hpp"
#include "dma_interface.hpp"
#include "memory_manager_interface.hpp"

/**
 * @brief DMA module which handles AXI bursts and sends data to the acceleration
 * modules.
 */
class DMA : public AccelerationModule, public DMAInterface {
 public:
  ~DMA() override = default;
  /**
   * @brief Constructor to setup the DMA module configuration class.
   * @param memory_manager Memory manager instance to access memory mapped
   * registers.
   */
  explicit DMA(MemoryManagerInterface* memory_manager)
      : AccelerationModule(memory_manager, 0){};

  /**
   * @brief Give input stream information to the input controller.
   * @param stream_id ID of the input stream.
   * @param ddr_burst_size How many clock cycles should the DDR burst be for the
   * input.
   * @param records_per_ddr_burst How many records are going to be transfered
   * during a DDR burst.
   * @param buffer_start Index of an input buffer where the stream will be
   * buffered to.
   * @param buffer_end Index of an input buffer up to where the stream can be
   * buffered to.
   */
  void SetInputControllerParams(int stream_id, int ddr_burst_size,
                                int records_per_ddr_burst, int buffer_start,
                                int buffer_end) override;
  /**
   * @brief Read input controller parameters.
   * @param stream_id ID of the stream to read.
   * @return Read configuration data.
   */
  auto GetInputControllerParams(int stream_id) -> volatile uint32_t override;
  /**
   * @brief Set the starting address of an input stream in DDR.
   * @param stream_id ID of the stream to be configured.
   * @param address Pointer to the starting point of the stream data.
   */
  void SetInputControllerStreamAddress(int stream_id,
                                       uintptr_t address) override;
  /**
   * @brief Read input stream's starting address.
   * @param stream_id ID of the stream.
   * @return Pointer to the start of the stream's data in DDR.
   */
  auto GetInputControllerStreamAddress(int stream_id)
      -> volatile uintptr_t override;
  /**
   * @brief Set how many records does the input stream have.
   * @param stream_id ID of the input stream.
   * @param size Number of records in the input stream.
   */
  void SetInputControllerStreamSize(int stream_id, int size) override;
  /**
   * @brief Read how many records does the input stream have.
   * @param stream_id ID of the input stream.
   * @return Number of records in the input stream.
   */
  auto GetInputControllerStreamSize(int stream_id) -> volatile int override;
  /**
   * @brief Start the input controller to start buffering input streams and
   * sending them to the acceleration modules.
   * @param stream_active Array of booleans showing which streams should start
   * streaming towards the modules.
   */
  void StartInputController(bool stream_active[16]) override;
  /**
   * @brief Find out if input controller has fully streamed all input streams.
   * @return Boolean showing if the input controller has finished.
   */
  auto IsInputControllerFinished() -> bool override;

  /**
   * @brief Set record size in terms of integers of a stream.
   * @param stream_id ID of the configured stream.
   * @param record_size How many integers of data does a record contain.
   */
  void SetRecordSize(int stream_id, int record_size) override;
  /**
   * @brief Give chunk IDs to different chunks of a record.
   * @param stream_id ID of the configured stream.
   * @param interface_cycle Which chunk is getting configured.
   * @param chunk_id The given ID.
   */
  void SetRecordChunkIDs(int stream_id, int interface_cycle,
                         int chunk_id) override;

  /**
   * @brief Give input stream information to the output controller.
   * @param stream_id ID of the output stream.
   * @param ddr_burst_size How many clock cycles should the DDR burst be for the
   * output.
   * @param records_per_ddr_burst How many records are going to be transfered
   * during an output DDR burst.
   * @param buffer_start Index of an output buffer where the stream will be
   * buffered to.
   * @param buffer_end Index of an input buffer up to where the stream can be
   * buffered to.
   */
  void SetOutputControllerParams(int stream_id, int ddr_burst_size,
                                 int records_per_ddr_burst, int buffer_start,
                                 int buffer_end) override;
  /**
   * @brief Read output controller parameters.
   * @param stream_id Output stream ID.
   * @return Data used to configure the given output stream.
   */
  auto GetOutputControllerParams(int stream_id) -> volatile uint32_t override;
  /**
   * @brief Set the starting address of an output stream in DDR.
   * @param stream_id Output stream ID.
   * @param address Pointer to the starting point of the stream data.
   */
  void SetOutputControllerStreamAddress(int stream_id,
                                        uintptr_t address) override;
  /**
   * @brief Read output stream's starting address.
   * @param stream_id Output stream ID.
   * @return Pointer to the start of the stream's data in DDR.
   */
  auto GetOutputControllerStreamAddress(int stream_id)
      -> volatile uintptr_t override;
  /**
   * @brief Set how many records does the output stream have.
   *
   * For output streams we can't know that so it's for setting the number to 0.
   * @param stream_id Output stream ID.
   * @param size Number of records in the output stream.
   */
  void SetOutputControllerStreamSize(int stream_id, int size) override;
  /**
   * Read how many records does the output stream have.
   * @param stream_id Output stream ID.
   * @return Number of records in the output stream.
   */
  auto GetOutputControllerStreamSize(int stream_id) -> volatile int override;
  /**
   * @brief Start the output controller to start pulling the streams through the
   * modules and write the results back to the DDR.
   *
   * More specifically the outputcontroller will start sending requrests of the
   * final results which will get propagated through the infrastructure.
   * @param stream_active Array of booleans showing which streams should start
   * getting pulled through the accelerators.
   */
  void StartOutputController(bool stream_active[16]) override;
  /**
   * @brief Find out if output controller has fully streamed all output streams.
   * @return Boolean which says if all streams have been processed.
   */
  auto IsOutputControllerFinished() -> bool override;

  /**
   * @brief Method to configure the input crossbar chunks.
   *
   * In-depth explanation is in the README.
   * @param stream_id Which input stream is being configured.
   * @param clock_cycle Which clock cycle is having its chunks set.
   * @param offset Which 4 chunks are being set.
   * @param source_chunk4 4th chunk
   * @param source_chunk3 3rd chunk
   * @param source_chunk2 2nd chunk
   * @param source_chunk1 1st chunk
   */
  void SetBufferToInterfaceChunk(int stream_id, int clock_cycle, int offset,
                                 int source_chunk4, int source_chunk3,
                                 int source_chunk2, int source_chunk1) override;
  /**
   * @brief Method to configure the input crossbar positions.
   *
   * In-depth explanation is in the README.
   * @param stream_id Input stream ID.
   * @param clock_cycle Clock cycle selection.
   * @param offset Configuration 4 position offset selection.
   * @param source_position4 4th position
   * @param source_position3 3rd position
   * @param source_position2 2nd position
   * @param source_position1 1st position
   */
  void SetBufferToInterfaceSourcePosition(int stream_id, int clock_cycle,
                                          int offset, int source_position4,
                                          int source_position3,
                                          int source_position2,
                                          int source_position1) override;

  /**
   * @brief Method to configure the output crossbar chunks.
   *
   * In-depth explanation is in the README.
   * @param stream_id Which output stream is being configured.
   * @param clock_cycle Which clock cycle is having its chunks set.
   * @param offset Which 4 chunks are being set.
   * @param target_chunk4 4th chunk
   * @param target_chunk3 3rd chunk
   * @param target_chunk2 2nd chunk
   * @param target_chunk1 1st chunk
   */
  void SetInterfaceToBufferChunk(int stream_id, int clock_cycle, int offset,
                                 int target_chunk4, int target_chunk3,
                                 int target_chunk2, int target_chunk1) override;
  /**
   * @brief Method to configure the output crossbar positions.
   *
   * In-depth explanation is in the README.
   * @param stream_id Output stream ID.
   * @param clock_cycle Clock cycle selection.
   * @param offset Configuration 4 position offset selection.
   * @param source_position4 4th position
   * @param source_position3 3rd position
   * @param source_position2 2nd position
   * @param source_position1 1st position
   */
  void SetInterfaceToBufferSourcePosition(int stream_id, int clock_cycle,
                                          int offset, int source_position4,
                                          int source_position3,
                                          int source_position2,
                                          int source_position1) override;

  /**
   * @brief Set the number of input streams which are going to have multiple
   * channels.
   * @param number Multichannel stream count.
   */
  void SetNumberOfInputStreamsWithMultipleChannels(int number) override;
  /**
   * @brief Set the number of records that will be streamed within a single
   * burst for multichannel streams.
   * @param stream_id Multichannel stream ID.
   * @param records_per_burst Number of records that will be streamed during a
   * DDR burst.
   */
  void SetRecordsPerBurstForMultiChannelStreams(int stream_id,
                                                int records_per_burst) override;
  /**
   * @brief Set the DDR burst length for the given multichannel stream.
   * @param stream_id Input multichannel stream ID.
   * @param ddr_burst_size How many clock cycles the DDR burst lasts.
   */
  void SetDDRBurstSizeForMultiChannelStreams(int stream_id,
                                             int ddr_burst_size) override;
  /**
   * @brief Set the number of used channels for the given multichannel stream.
   * @param stream_id Input multichannels stream ID.
   * @param active_channels How many active channels there are for this stream.
   */
  void SetNumberOfActiveChannelsForMultiChannelStreams(
      int stream_id, int active_channels) override;
  /**
   * @brief Set the starting address of a stream's channel.
   * @param stream_id Multichannel stream ID.
   * @param channel_id Channel ID.
   * @param address Pointer to the start of the stream's data in DDR.
   */
  void SetAddressForMultiChannelStreams(int stream_id, int channel_id,
                                        uintptr_t address) override;
  /**
   * @brief Set how many records should be streamed into the given channel of
   * the given stream.
   * @param stream_id Multichannel stream ID.
   * @param channel_id Stream's channel ID.
   * @param number_of_records Number of records to be streamed into the given
   * channel.
   */
  void SetSizeForMultiChannelStreams(int stream_id, int channel_id,
                                     int number_of_records) override;

  /**
   * @brief Debug method to get the runtime of the run. Measured by the
   * hardware.
   * @return Amount of clock cycles spent on the execution of the query node.
   */
  auto GetRuntime() -> volatile uint64_t override;
  /**
   * @brief Debug method to read the valid read cycle counter register.
   * @return Number of cycles there were valid reads.
   */
  auto GetValidReadCyclesCount() -> volatile uint64_t override;
  /**
   * @brief Debug method to read the valid write cycle counter register.
   * @return Number of cycles there were valid writes.
   */
  auto GetValidWriteCyclesCount() -> volatile uint64_t override;

  /**
   * @brief Method to write to the reset register to reset all configured
   * modules including the DMA.
   *
   * The reset signal will be high for 8 clock cycles.
   */
  void GlobalReset() override;
};
