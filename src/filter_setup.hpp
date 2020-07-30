#pragma once
#include "filter.hpp"
class FilterSetup {
 public:
  static void SetupFilterModule(FilterInterface& filter_module,
                                int input_stream_id, int output_stream_id);
};
