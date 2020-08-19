#pragma once
#include <cstdint>
class FilterInterface {
 public:
  virtual ~FilterInterface() = default;
  ;

  virtual void FilterSetStreamIDs(int stream_id_input,
                                  int stream_id_valid_output,
                                  int stream_id_invalid_output) = 0;
  virtual void FilterSetMode(bool request_on_invalid_if_last,
                             bool forward_invalid_record_first_chunk,
                             bool forward_full_invalid_records,
                             bool first_module_in_resource_elastic_chain,
                             bool last_module_in_resource_elastic_chain) = 0;
  virtual void FilterSetCompareTypes(int chunk_id, int data_position,
                                     int compare_1_type, int compare_2_type,
                                     int compare_3_type,
                                     int compare_4_type) = 0;
  virtual void FilterSetCompareReferenceValue(
      int chunk_id, int data_position, int compare_number,
                                              int compare_reference_value) = 0;
  virtual void FilterSetDNFClauseLiteral(int dnf_clause_id, int compare_number,
                                         int chunk_id, int data_position,
                                         int literal_type) = 0;

  virtual void WriteDNFClauseLiteralsToFilter_1CMP_8DNF(int datapath_width) = 0;
  virtual void WriteDNFClauseLiteralsToFilter_2CMP_16DNF(
      int datapath_width) = 0;
  virtual void WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
      int datapath_width) = 0;
};