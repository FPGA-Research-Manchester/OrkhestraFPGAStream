#include "filter.hpp"

#include<iostream>
// Filter module low driver

// Selects stream_id and stream_id manipulations
void Filter::FilterSetStreamIDs(
    int stream_id_input,  // The stream_id of the stream that gets filterred
    int stream_id_valid_output,  // The stream_id of valid output from filters
    int stream_id_invalid_output) {  // The stream_id of invalid output
                                     // from filters
  AccelerationModule::WriteToModule(0, stream_id_input +
                                           (stream_id_valid_output << 8) +
                                           (stream_id_invalid_output << 16));
}

// Selects mode of operation
void Filter::FilterSetMode(
    bool request_on_invalid_if_last,  // Should the module rerequest packets if
                                      // outcome is invalid? (Only applicable
                                      // for last module in RE chain)
    bool forward_invalid_record_first_chunk,  // Should the module forward the
                                              // first chunk of invalid records?
                                              // (Only applicable for last
                                              // module in RE chain)
    bool forward_full_invalid_records,  // Should the module forward all chunks
                                        // of invalid records? (Only applicable
                                        // for last module in RE chain)
    bool
        first_module_in_resource_elastic_chain,  // Indicating this is the first
                                                 // filter module (must be true
                                                 // if only 1 module is used)
    bool
        last_module_in_resource_elastic_chain) {  // Indicating this is the last
  // filter module  (must be true if
  // only 1 module is used)
  AccelerationModule::WriteToModule(
      0x4, (request_on_invalid_if_last ? 16 : 0) +
               (forward_invalid_record_first_chunk ? 8 : 0) +
               (forward_full_invalid_records ? 4 : 0) +
               (first_module_in_resource_elastic_chain ? 2 : 0) +
               (last_module_in_resource_elastic_chain ? 1 : 0));
}

// Every 32-bit integer on any position and any chunk ID can have a different
// compare type and different reference value Moreover, they can have more than
// one compare type and reference value (we enumerate them as CMP0, CMP1, CMP2
// ...)
void Filter::FilterSetCompareTypes(
    int chunk_id,       // for which chunkID are the following compare types
    int data_position,  // which 32-bit integer are the following compare
                        // types for
    filter_config_values::CompareFunctions
        compare_1_type,  // The type of comparison for each of up to 4 different
    filter_config_values::CompareFunctions
        compare_2_type,  // compares (depending on module type, e.g. for
                         // 1-Compare module, only Compare_1_Type will be used)
    filter_config_values::CompareFunctions
        compare_3_type,  // Compare types are defined above (e.g.
                         // FILTER_32BIT_LESS_THAN)
    filter_config_values::CompareFunctions
        compare_4_type) {  // In 64-bit compares, the current 32-bit integer
                           // holds the MSBits, while (DataPosition-1) holds the
                           // Least significant bits (reference value)
  AccelerationModule::WriteToModule(
      ((1 << 15) + (data_position << 2) + (chunk_id << 7)),
      ((static_cast<int>(compare_4_type)
       << 12) + (static_cast<int>(compare_3_type)
       << 8) + (static_cast<int>(compare_2_type)
       << 4) + static_cast<int>(compare_1_type)));
}

void Filter::FilterSetCompareReferenceValue(
    int chunk_id,  // for which chunkID is the following compare reference value
    int data_position,       // for which 32-bit integer is the following
                             // compare reference value
    int compare_lane_index,  // Which CMP is this reference value for (i.e., 0,
                             // 1, 2, 3. Module with only 2 Compares per field
                             // can take compare_number of 1 and 2)
    int compare_reference_value) {  // The 32-bit value we compare against.
                                    // Can be anything (Can be 4 characters
                                    // of text, can be a float number for
                                    // equal compare etc.)
  AccelerationModule::WriteToModule(
      ((1 << 15) + (data_position << 2) + (chunk_id << 7) +
       ((compare_lane_index + 1) << 12)),
      compare_reference_value);
}

void Filter::FilterSetDNFClauseLiteral(
    int dnf_clause_id /*0-31*/, int compare_number /*0-3*/,
    int chunk_id /*0-31*/, int data_position /*0-15 for 512-bit datapath etc*/,
    filter_config_values::LiteralTypes literal_type) {
  dnf_states[dnf_clause_id].first = true; // DNF clause is used
  dnf_states[dnf_clause_id].second[compare_number][chunk_id][data_position] =
      literal_type;
}

void Filter::FilterWriteDNFClauseLiteralsToModule(int datapath_width,
                                                  int module_compares_per_field,
                                                  int module_dnf_clauses) {
  int compare_lane = 0;
  int dnf_clause_index = 0;
  int data_position = 0;
  int chunk_id = 0;
  for (compare_lane = 0; compare_lane < module_compares_per_field;
       compare_lane++) {
    for (data_position = 0; data_position < datapath_width; data_position++) {
      for (chunk_id = 0; chunk_id < 32; chunk_id++) {
        int clauses_packed_positive_result = 0;
        int clauses_packed_negative_result = 0;
        for (dnf_clause_index = module_dnf_clauses - 1; dnf_clause_index >= 0;
             dnf_clause_index--) {
          clauses_packed_positive_result <<= 1;
          clauses_packed_negative_result <<= 1;
          if (dnf_states[dnf_clause_index].first) {
            if (dnf_states[dnf_clause_index]
                    .second[compare_lane][chunk_id]
                           [data_position] ==
                filter_config_values::LiteralTypes::kLiteralDontCare) {
              clauses_packed_positive_result |= 1;
              clauses_packed_negative_result |= 1;
            } else if (dnf_states[dnf_clause_index]
                           .second[compare_lane][chunk_id][data_position] ==
                       filter_config_values::LiteralTypes::kLiteralPositive) {
              clauses_packed_positive_result |= 1;
              clauses_packed_negative_result |= 0;
            } else if (dnf_states[dnf_clause_index]
                           .second[compare_lane][chunk_id][data_position] ==
                       filter_config_values::LiteralTypes::kLiteralNegative) {
              clauses_packed_positive_result |= 0;
              clauses_packed_negative_result |= 1;
            }
          } else {                                // else DNF clause is unused
            clauses_packed_positive_result |= 0;  // therefore it cannot satisfy
            clauses_packed_negative_result |= 0;  // boolean expression
          }
        }
        AccelerationModule::WriteToModule(
            ((1 << 16) + (data_position << 2) + (1 << 7) + (chunk_id << 8) +
             (compare_lane << 13)),
            clauses_packed_positive_result);
        AccelerationModule::WriteToModule(
            ((1 << 16) + (data_position << 2) + (0 << 7) + (chunk_id << 8) +
             (compare_lane << 13)),
            clauses_packed_negative_result);
      }
    }
  }
}
void Filter::WriteDNFClauseLiteralsToFilter_1CMP_8DNF(
    int datapath_width /*1-32: 16->512bit datapath; 32->1024-bit datapath*/) {
  FilterWriteDNFClauseLiteralsToModule(datapath_width, 1, 8);
}
void Filter::WriteDNFClauseLiteralsToFilter_2CMP_16DNF(
    int datapath_width /*1-32: 16->512bit datapath; 32->1024-bit datapath*/) {
  FilterWriteDNFClauseLiteralsToModule(datapath_width, 2, 16);
}
void Filter::WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
    int datapath_width /*1-32: 16->512bit datapath; 32->1024-bit datapath*/) {
  FilterWriteDNFClauseLiteralsToModule(datapath_width, 4, 32);
}

void Filter::ResetDNFStates() {
  dnf_states = filter_config_values::DNFClauseStates();
}
