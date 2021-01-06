#pragma once
#include "linear_sort_interface.hpp"
class LinearSortSetup {
 private:
 public:
  static void SetupLinearSortModule(LinearSortInterface& linear_sort_module,
                                    int stream_id, int record_size);
};