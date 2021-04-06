#pragma once
#include "linear_sort_interface.hpp"
/**
 * Linear sort setup class which will calculate the configuration data to setup
 * the module
 */
class LinearSortSetup {
 private:
 public:
  /**
   * Setup linear sort module by giving the stream data to be sorted.
   * @param linear_sort_module Module instance to access the configuration
   *  registers.
   * @param stream_id ID of the stream to be sorted.
   * @param record_size How many integers worth of data does a record have.
   */
  static void SetupLinearSortModule(LinearSortInterface& linear_sort_module,
                                    int stream_id, int record_size);
};