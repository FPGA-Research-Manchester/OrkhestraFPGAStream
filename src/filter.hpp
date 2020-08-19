#pragma once
#include <cstdint>

#include "acceleration_module.hpp"
#include "filter_interface.hpp"
class Filter : public AccelerationModule, public FilterInterface {
 private:
  void FilterWriteDNFClauseLiteralsToModule(int datapath_width,
                                            int module_compares_per_field,
                                            int module_dnf_clauses);

 public:
  ~Filter() override;
  Filter(volatile int* ctrl_axi_base_address, int module_position);

  void FilterSetStreamIDs(int stream_id_input, int stream_id_valid_output,
                          int stream_id_invalid_output) override;
  void FilterSetMode(bool request_on_invalid_if_last,
                     bool forward_invalid_record_first_chunk,
                     bool forward_full_invalid_records,
                     bool first_module_in_resource_elastic_chain,
                     bool last_module_in_resource_elastic_chain) override;
  void FilterSetCompareTypes(int chunk_id, int data_position,
                             int compare_1_type, int compare_2_type,
                             int compare_3_type, int compare_4_type) override;
  void FilterSetCompareReferenceValue(
      int chunk_id, int data_position, int compare_lane_index,
                                      int compare_reference_value) override;
  void FilterSetDNFClauseLiteral(int dnf_clause_id, int compare_number,
                                 int chunk_id, int data_position,
                                 int literal_type) override;

  void WriteDNFClauseLiteralsToFilter_1CMP_8DNF(
      int datapath_width) override;
  void WriteDNFClauseLiteralsToFilter_2CMP_16DNF(
      int datapath_width) override;
  void WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
      int datapath_width) override;
};
