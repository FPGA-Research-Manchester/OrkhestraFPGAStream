#include "join.hpp"

Join::~Join() {}

Join::Join(StaticAccelInst* acceleration_instance, int module_position)
    : AccelerationModule(acceleration_instance, module_position) {}

void Join::DefineOutputStream(int output_stream_chunk_count,
                              int first_input_stream_id,
                              int second_input_stream_id,
                              int output_stream_id) {}

void Join::SetFirstInputStreamChunkCount(int chunk_count) {}

void Join::SetSecondInputStreamChunkCount(int chunk_count) {}

void Join::SelectNextOutputDataElement(int output_chunk_id, int input_chunk_id,
                                       int data_position,
                                       bool is_element_from_first_stream) {}
