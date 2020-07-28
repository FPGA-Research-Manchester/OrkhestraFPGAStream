#pragma once
#include <cstdint>
class FilterInterface {
 public:
  virtual ~FilterInterface() = default;
  ;

  virtual void filterSetStreamIDs(uint32_t stream_id_input,
                                  uint32_t stream_id_valid_output,
                                  uint32_t stream_id_invalid_output) = 0;
  virtual void filterSetMode(bool request_on_invalid_if_last,
                             bool forward_invalid_record_first_chunk,
                             bool forward_full_invalid_records,
                             bool first_module_in_resource_elastic_chain,
                             bool last_module_in_resource_elastic_chain) = 0;
  virtual void filterSetCompareTypes(uint32_t chunk_id, uint32_t data_position,
                                     uint32_t compare_1_type,
                                     uint32_t compare_2_type,
                                     uint32_t compare_3_type,
                                     uint32_t compare_4_type) = 0;
  virtual void filterSetCompareReferenceValue(
      uint32_t chunk_id, uint32_t data_position, uint32_t compare_number,
      uint32_t compare_reference_value) = 0;
  virtual void filterSetDNFClauseLiteral(uint32_t dnf_clause_id,
                                         uint32_t compare_number,
                                         uint32_t chunk_id,
                                         uint32_t data_position,
                                         uint8_t literal_type) = 0;

  virtual void writeDNFClauseLiteralsToFilter_1CMP_8DNF(
      uint32_t datapath_width) = 0;
  virtual void writeDNFClauseLiteralsToFilter_2CMP_16DNF(
      uint32_t datapath_width) = 0;
  virtual void writeDNFClauseLiteralsToFilter_4CMP_32DNF(
      uint32_t datapath_width) = 0;
};