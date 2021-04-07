#include "linear_sort_setup.hpp"
#include "stream_parameter_calculator.hpp"

using namespace dbmstodspi::fpga_managing;

void LinearSortSetup::SetupLinearSortModule(LinearSortInterface& linear_sort_module,
                                int stream_id, int record_size) {
  int chunks_per_record =
      StreamParameterCalculator::CalculateChunksPerRecord(record_size);

  linear_sort_module.SetStreamParams(stream_id, chunks_per_record);

  linear_sort_module.StartPrefetchingData();
}