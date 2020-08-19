#pragma once
#include <cstdint>

#include "filter_interface.hpp"
#include "gmock/gmock.h"

class MockFilter : public FilterInterface {
 public:
  MOCK_METHOD(void, FilterSetStreamIDs,
              (int stream_id_input, int stream_id_valid_output,
               int stream_id_invalid_output),
              (override));
  MOCK_METHOD(void, FilterSetMode,
              (bool request_on_invalid_if_last,
               bool forward_invalid_record_first_chunk,
               bool forward_full_invalid_records,
               bool first_module_in_resource_elastic_chain,
               bool last_module_in_resource_elastic_chain),
              (override));
  MOCK_METHOD(void, FilterSetCompareTypes,
              (int chunk_id, int data_position, int compare_1_type,
               int compare_2_type, int compare_3_type, int compare_4_type),
              (override));
  MOCK_METHOD(void, FilterSetCompareReferenceValue,
              (int chunk_id, int data_position, int compare_index,
               int compare_reference_value),
              (override));
  MOCK_METHOD(void, FilterSetDNFClauseLiteral,
              (int dnf_clause_id, int compare_index, int chunk_id,
               int data_position, int literal_type),
              (override));

  MOCK_METHOD(void, WriteDNFClauseLiteralsToFilter_1CMP_8DNF,
              (int datapath_width), (override));
  MOCK_METHOD(void, WriteDNFClauseLiteralsToFilter_2CMP_16DNF,
              (int datapath_width), (override));
  MOCK_METHOD(void, WriteDNFClauseLiteralsToFilter_4CMP_32DNF,
              (int datapath_width), (override));
};