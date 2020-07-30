#pragma once
#include <cstdint>

#include "FilterInterface.hpp"
#include "gmock/gmock.h"

class MockFilter : public FilterInterface {
 public:
  MOCK_METHOD(void, FilterSetStreamIDs,
              (uint32_t stream_id_input, uint32_t stream_id_valid_output,
               uint32_t stream_id_invalid_output),
              (override));
  MOCK_METHOD(void, FilterSetMode,
              (bool request_on_invalid_if_last, bool forward_invalid_record_first_chunk,
               bool forward_full_invalid_records,
               bool first_module_in_resource_elastic_chain,
               bool last_module_in_resource_elastic_chain),
              (override));
  MOCK_METHOD(void, FilterSetCompareTypes,
              (uint32_t chunk_id, uint32_t data_position, uint32_t compare_1_type,
               uint32_t compare_2_type, uint32_t compare_3_type,
               uint32_t compare_4_type),
              (override));
  MOCK_METHOD(void, FilterSetCompareReferenceValue,
              (uint32_t chunk_id, uint32_t data_position, uint32_t compare_index,
               uint32_t compare_reference_value),
              (override));
  MOCK_METHOD(void, FilterSetDNFClauseLiteral,
              (uint32_t dnf_clause_id, uint32_t compare_index, uint32_t chunk_id,
               uint32_t data_position, uint8_t literal_type),
              (override));

  MOCK_METHOD(void, WriteDNFClauseLiteralsToFilter_1CMP_8DNF,
              (uint32_t datapath_width), (override));
  MOCK_METHOD(void, WriteDNFClauseLiteralsToFilter_2CMP_16DNF,
              (uint32_t datapath_width), (override));
  MOCK_METHOD(void, WriteDNFClauseLiteralsToFilter_4CMP_32DNF,
              (uint32_t datapath_width), (override));
};