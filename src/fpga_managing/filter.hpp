#pragma once
#include <cstdint>

#include "acceleration_module.hpp"
#include "filter_config_values.hpp"
#include "filter_interface.hpp"
#include "memory_manager_interface.hpp"

/**
 * Class which implements the filtering operation acceleration module.
 */
class Filter : public AccelerationModule, public FilterInterface {
 private:
  filter_config_values::DNFClauseStates dnf_states_;
  void FilterWriteDNFClauseLiteralsToModule(int datapath_width,
                                            int module_compares_per_field,
                                            int module_dnf_clauses);

 public:
  ~Filter() override = default;
  /**
   * Constructor to setup the memory mapped register space.
   * @param memory_manager Memory manager to access memory mapped configuration
   *  registers.
   * @param module_position Position of the module on the FPGA.
   */
  explicit Filter(MemoryManagerInterface* memory_manager, int module_position)
      : AccelerationModule(memory_manager, module_position){};

  /**
   * Set filter input and output stream IDs.
   * @param stream_id_input Input ID.
   * @param stream_id_valid_output Valid output ID.
   * @param stream_id_invalid_output Invalid output ID.
   */
  void FilterSetStreamIDs(int stream_id_input, int stream_id_valid_output,
                          int stream_id_invalid_output) override;
  /**
   * Set filter location and mode parameters.
   * @param request_on_invalid_if_last If last module in chain, should it
   *  request new records on invalid.
   * @param forward_invalid_record_first_chunk If last, forward first chunk of
   *  invalid records.
   * @param forward_full_invalid_records If last, forward full invalid records.
   * @param first_module_in_resource_elastic_chain Is this module first in
   *  filter chain.
   * @param last_module_in_resource_elastic_chain Is this module last in filter
   *  chain.
   */
  void FilterSetMode(bool request_on_invalid_if_last,
                     bool forward_invalid_record_first_chunk,
                     bool forward_full_invalid_records,
                     bool first_module_in_resource_elastic_chain,
                     bool last_module_in_resource_elastic_chain) override;
  /**
   * Set which comparison functions are used in each of the compare lanes.
   * @param chunk_id Which chunk comparison is being configured.
   * @param data_position Which position comparison is being configured.
   * @param compare_1_type 1st comparison lane compare function.
   * @param compare_2_type 2nd comparison lane compare function.
   * @param compare_3_type 3rd comparison lane compare function.
   * @param compare_4_type 4th comparison lane compare function.
   */
  void FilterSetCompareTypes(
      int chunk_id, int data_position,
      filter_config_values::CompareFunctions compare_1_type,
      filter_config_values::CompareFunctions compare_2_type,
      filter_config_values::CompareFunctions compare_3_type,
      filter_config_values::CompareFunctions compare_4_type) override;
  /**
   * Set comparison reference values for specific comparitors.
   * @param chunk_id Which chunk compariosn is being configured.
   * @param data_position Which position comparison is being configured.
   * @param compare_lane_index Which compare lane is being configured.
   * @param compare_reference_value Comparison reference value.
   */
  void FilterSetCompareReferenceValue(int chunk_id, int data_position,
                                      int compare_lane_index,
                                      int compare_reference_value) override;
  /**
   * Set DNF clause literal type.
   * @param dnf_clause_id Clause ID.
   * @param compare_number Compare lane number.
   * @param chunk_id Chunk ID.
   * @param data_position Position selection.
   * @param literal_type If the literal should be positive or negative.
   */
  void FilterSetDNFClauseLiteral(
      int dnf_clause_id, int compare_number, int chunk_id, int data_position,
      filter_config_values::LiteralTypes literal_type) override;

  /**
   * Write the set DNF clause literal types to the module which has 1 compare
   *  lane and 8 DNF clauses.
   * @param datapath_width How wide is the data in integers.
   */
  void WriteDNFClauseLiteralsToFilter_1CMP_8DNF(int datapath_width) override;
  /**
   * Write the set DNF clause literal types to the module which has 2 compare
   *  lane and 16 DNF clauses.
   * @param datapath_width How wide is the data in integers.
   */
  void WriteDNFClauseLiteralsToFilter_2CMP_16DNF(int datapath_width) override;
  /**
   * Write the set DNF clause literal types to the module which has 4 compare
   *  lane and 32 DNF clauses.
   * @param datapath_width How wide is the data in integers.
   */
  void WriteDNFClauseLiteralsToFilter_4CMP_32DNF(int datapath_width) override;

  /**
   * Reset the DNF state data for a new configuration for this filter module
   *  instance.
   */
  void ResetDNFStates() override;
};
