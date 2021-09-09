/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once
#include <cstdint>

#include "filter_interface.hpp"
#include "gmock/gmock.h"

using easydspi::dbmstodspi::FilterInterface;
using easydspi::dbmstodspi::module_config_values::FilterCompareFunctions;
using easydspi::dbmstodspi::module_config_values::LiteralTypes;

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
              (int chunk_id, int data_position,
               FilterCompareFunctions compare_1_type,
               FilterCompareFunctions compare_2_type,
               FilterCompareFunctions compare_3_type,
               FilterCompareFunctions compare_4_type),
              (override));
  MOCK_METHOD(void, FilterSetCompareReferenceValue,
              (int chunk_id, int data_position, int compare_index,
               int compare_reference_value),
              (override));
  MOCK_METHOD(void, FilterSetDNFClauseLiteral,
              (int dnf_clause_id, int compare_index, int chunk_id,
               int data_position, LiteralTypes literal_type),
              (override));

  MOCK_METHOD(void, WriteDNFClauseLiteralsToFilter_1CMP_8DNF,
              (int datapath_width), (override));
  MOCK_METHOD(void, WriteDNFClauseLiteralsToFilter_2CMP_16DNF,
              (int datapath_width), (override));
  MOCK_METHOD(void, WriteDNFClauseLiteralsToFilter_4CMP_32DNF,
              (int datapath_width), (override));

  MOCK_METHOD(void, ResetDNFStates, (), (override));
};