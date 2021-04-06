#pragma once
#include "join_interface.hpp"
/**
 * Class to calculate the join module configuration data and write the data to
 * the registers.
 */
class JoinSetup {
 private:
  static void SetupTimeMultiplexer(JoinInterface& join_module,
                                   int first_stream_size,
                                   int second_stream_size, int shift_size);

 public:
  /**
   * Method to setup the join module. The first stream will come first and the
   *  streams will be joined with the first element of both records. The second
   *  stream will have to be shifted to avoid overwriting data.
   * @param join_module Join instance to access memory mapped configuration
   *  registers.
   * @param first_input_stream_id ID of first input stream.
   * @param first_input_record_size How many integers worth of data is there in
   *  the first stream.
   * @param second_input_stream_id ID of the second input stream.
   * @param second_input_record_size How many integers worth of data is there in
   *  the second stream.
   * @param output_stream_id Output stream ID.
   * @param output_chunks_per_record How many chunks will be used for the output
   *  stream.
   * @param shift_size How much is the second stream shifted.
   */
  static void SetupJoinModule(JoinInterface& join_module,
                              int first_input_stream_id,
                              int first_input_record_size,
                              int second_input_stream_id,
                              int second_input_record_size,
                              int output_stream_id,
                              int output_chunks_per_record, int shift_size);
};