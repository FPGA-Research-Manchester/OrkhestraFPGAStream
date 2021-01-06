#include "linear_sort.hpp"

void LinearSort::StartPrefetchingData() {
  AccelerationModule::WriteToModule(0, 1);
}

void LinearSort::SetStreamParams(int stream_id, int chunks_per_record) {
  AccelerationModule::WriteToModule(4,
                                    ((chunks_per_record -1 << 8) + stream_id));
}