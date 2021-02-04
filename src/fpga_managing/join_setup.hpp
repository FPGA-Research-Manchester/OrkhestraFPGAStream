#pragma once
#include "join_interface.hpp"
class JoinSetup {
 private:
  static void SetupTimeMultiplexer(JoinInterface& join_module,
                                   int first_stream_size,
                                   int second_stream_size,
                                   int output_stream_size);

 public:
  static void SetupJoinModule(JoinInterface& join_module,
                              int first_input_stream_id,
                              int first_input_record_size,
                              int second_input_stream_id,
                              int second_input_record_size,
                              int output_stream_id, int output_record_size);
};