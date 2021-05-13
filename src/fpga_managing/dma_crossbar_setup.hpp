#pragma once
#include <queue>
#include <vector>

#include "dma_setup_data.hpp"

namespace dbmstodspi::fpga_managing {

/**
 * @brief Class to write the correct setup data to the DMASetupData struct
 * according to the specification.
 */
class DMACrossbarSetup {
 public:
  /**
   * @brief Calculate crossbar setup data such that the configuration data can
   * be directly written to the DMA registers to configure the crossbar
   * according to the the given specifications.
   * @param stream_setup_data Setup data struct where the DMA configuration is
   * stored.
   * @param record_size How many integers worth of data there is in a record.
   * @param selected_columns Integer vector noting which columns should be
   * where.
   */
  static void CalculateCrossbarSetupData(
      DMASetupData& stream_setup_data, int record_size,
      const std::vector<int>& selected_columns);

 private:
  static auto GetReverseIndex(int index, int row_size) -> int;

  static void SetUpEmptyCrossbarSetupData(DMASetupData& stream_setup_data,
                                          int required_chunk_count);

  static void FillSetupDataWithNegativePositions(
      DMASetupData& stream_setup_data);

  static void SetNextInputConfiguration(DMASetupData& stream_setup_data,
                                        int current_location,
                                        int target_location);
  static void SetNextOutputConfiguration(DMASetupData& stream_setup_data,
                                         int current_location,
                                         int target_location);

  static auto CreateFreeChunksVector() -> std::vector<int>;
  static void MarkChunksAsUsed(const DMASetupData& stream_setup_data,
                               int column_id, std::vector<int>& free_chunks);
  static void AllocateAvailableChunks(DMASetupData& stream_setup_data,
                                      int column_id,
                                      const std::vector<int>& free_chunks);
  static void InsertMissingEmptySetupChunks(DMASetupData& stream_setup_data,
                                            int missing_chunk_count_per_record);

  static void PrintCrossbarConfigData(
      const std::vector<int>& expanded_column_selection,
      const DMASetupData& stream_setup_data);

  static void ConfigureInputCrossbarSetupData(
      const std::vector<int>& selected_columns, DMASetupData& stream_setup_data,
      std::vector<int>& expanded_column_selection, const int& record_size);
  static void ConfigureOutputCrossbarSetupData(
      const std::vector<int>& selected_columns,
      std::vector<int>& expanded_column_selection,
      DMASetupData& stream_setup_data);
};

}  // namespace dbmstodspi::fpga_managing
