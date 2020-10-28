#pragma once
#include "join_interface.hpp"
class JoinSetup {
 private:
  static void SetupTimeMultiplexer(JoinInterface& join_module);
 public:
  static void SetupJoinModule(JoinInterface& join_module,
                                int first_input_stream_id, int second_input_stream_id, int output_stream_id);
};