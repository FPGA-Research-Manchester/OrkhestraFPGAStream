#include "Filter.hpp"
// Filter module low driver

#define MODULE_ADDRESS_BITS 20

Filter::~Filter() = default;

Filter::Filter(int* volatile ctrl_ax_ibase_address, uint32_t module_position)
    : AccelerationModule(ctrl_ax_ibase_address, module_position) {}

// Selects streamID and streamID manipulations
void Filter::FilterSetStreamIDs(
    uint32_t stream_id_input,  // The streamID of the stream that gets filterred
    uint32_t
        stream_id_valid_output,  // The streamID of valid output from filters
    uint32_t stream_id_invalid_output) {  // The streamID of invalid output from
                                          // filters
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
#define FILTER_32BIT_LESS_THAN 0
#define FILTER_32BIT_LESS_THAN_OR_EQUAL 1
#define FILTER_32BIT_EQUAL 2
#define FILTER_32BIT_GREATER_THAN_OR_EQUAL 3
#define FILTER_32BIT_GREATER_THAN 4
#define FILTER_32BIT_NOT_EQUAL 5

#define FILTER_64BIT_LESS_THAN 8
#define FILTER_64BIT_LESS_THAN_OR_EQUAL 9
#define FILTER_64BIT_EQUAL 10
#define FILTER_64BIT_GREATER_THAN_OR_EQUAL 11
#define FILTER_64BIT_GREATER_THAN 12
#define FILTER_64BIT_NOT_EQUAL 13

void Filter::FilterSetCompareTypes(
    uint32_t chunk_id,  // for which chunkID are the following compare types
    uint32_t data_position,  // which 32-bit integer are the following compare
                             // types for

    uint32_t
        compare_1_type,  // The type of comparison for each of up to 4 different
    uint32_t
        compare_2_type,  // compares (depending on module type, e.g. for
                         // 1-Compare module, only Compare_1_Type will be used)
    uint32_t compare_3_type,  // Compare types are defined above (e.g.
                              // FILTER_32BIT_LESS_THAN)
    uint32_t
        compare_4_type) {  // In 64-bit compares, the current 32-bit integer
                           // holds the MSBits, while (DataPosition-1) holds the
                           // Least significant bits (reference value)
  AccelerationModule::WriteToModule(
      ((1 << 15) + (data_position << 2) + (chunk_id << 7)),
      (compare_4_type << 12 + compare_3_type << 8 + compare_2_type
                      << 4 + compare_1_type));
}

void Filter::FilterSetCompareReferenceValue(
    uint32_t
        chunk_id,  // for which chunkID is the following compare reference value
    uint32_t data_position,  // for which 32-bit integer is the following
                             // compare reference value

    uint32_t compare_number,  // Which CMP is this reference value for (i.e., 1,
                              // 2, 3, 4. Module with only 2 Compares per field
                              // can take CompareNumber of 1 and 2)

    uint32_t compare_reference_value) {  // The 32-bit value we compare against.
                                         // Can be anything (Can be 4 characters
                                         // of text, can be a float number for
                                         // equal compare etc.)
  AccelerationModule::WriteToModule(((1 << 15) + (data_position << 2) +
                                     (chunk_id << 7) + (compare_number << 12)),
                                    compare_reference_value);
}

#define LITERAL_DONT_CARE 0
#define LITERAL_POSITIVE 1
#define LITERAL_NEGATIVE 2
struct {
  bool DNF_is_used = false;
  struct {
    struct {
      struct {
        uint8_t literalState =
            LITERAL_DONT_CARE;  // 0 - literal is not present in the DNF clause;
                                // 1 - positive literal in the DNF clause ; 2 -
                                // negative literal in the DNF clause
      } DataPosition[32];  // Up to 32 data positions of integers (32-bit) for a
                           // maximum 1024-bit datapath (for 512-bit datapath
                           // use 0-15 only)
    } ChunkID[32];         // Every chunkID can produce different literals
  } CompareNumber[4];  // Up to four different compares with different reference
                       // values per 32-bit field (for 2 compares per field use
                       // 0-1 only)
} dnf_clause[32];  // Up to 32 Clauses (for 16 DNF clause module use 0-15 only)

void Filter::FilterSetDNFClauseLiteral(
    uint32_t dnf_clause_id /*0-31*/, uint32_t compare_number /*1-4*/,
    uint32_t chunk_id /*0-31*/,
    uint32_t data_position /*0-15 for 512-bit datapath etc*/,
    uint8_t literal_type) {
  dnf_clause[dnf_clause_id].DNF_is_used = true;
  dnf_clause[dnf_clause_id]
      .CompareNumber[compare_number]
      .ChunkID[chunk_id]
      .DataPosition[data_position]
      .literalState = literal_type;
}

void Filter::filterWriteDNFClauseLiteralsToModule(
    uint32_t datapath_width, uint32_t module_compares_per_field,
    uint32_t module_dnf_clauses) {
  int compare_lane = 0;
  int dnf_clause_index = 0;
  int data_position = 0;
  int chunk_id = 0;
  for (compare_lane = 0; compare_lane < module_compares_per_field;
       compare_lane++) {
    for (data_position = 0; data_position < datapath_width; data_position++) {
      for (chunk_id = 0; chunk_id < 32; chunk_id++) {
        uint32_t clauses_packed_positive_result = 0xFFFFFFFF;
        uint32_t clauses_packed_negative_result = 0xFFFFFFFF;
        for (dnf_clause_index = 0; dnf_clause_index < module_dnf_clauses;
             dnf_clause_index++) {
          if (dnf_clause[dnf_clause_index].DNF_is_used) {
            clauses_packed_positive_result <<= 1;
            clauses_packed_negative_result <<= 1;
            if (dnf_clause[dnf_clause_index]
                    .CompareNumber[compare_lane]
                    .ChunkID[chunk_id]
                    .DataPosition[data_position]
                    .literalState == LITERAL_DONT_CARE) {
              clauses_packed_positive_result |= 1;
              clauses_packed_negative_result |= 1;
            } else if (dnf_clause[dnf_clause_index]
                           .CompareNumber[compare_lane]
                           .ChunkID[chunk_id]
                           .DataPosition[data_position]
                           .literalState == LITERAL_POSITIVE) {
              clauses_packed_positive_result |= 1;
              clauses_packed_negative_result |= 0;
            } else if (dnf_clause[dnf_clause_index]
                           .CompareNumber[compare_lane]
                           .ChunkID[chunk_id]
                           .DataPosition[data_position]
                           .literalState == LITERAL_NEGATIVE) {
              clauses_packed_positive_result |= 0;
              clauses_packed_negative_result |= 1;
            }
          } else {                               // else DNF clause is unused
            clauses_packed_positive_result = 0;  // therefore it cannot satisfy
            clauses_packed_negative_result = 0;  // boolean expression
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
    uint32_t
        datapath_width /*1-32: 16->512bit datapath; 32->1024-bit datapath*/) {
  filterWriteDNFClauseLiteralsToModule(datapath_width, 1, 8);
}
void Filter::WriteDNFClauseLiteralsToFilter_2CMP_16DNF(
    uint32_t
        datapath_width /*1-32: 16->512bit datapath; 32->1024-bit datapath*/) {
  filterWriteDNFClauseLiteralsToModule(datapath_width, 2, 16);
}
void Filter::WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
    uint32_t
        datapath_width /*1-32: 16->512bit datapath; 32->1024-bit datapath*/) {
  filterWriteDNFClauseLiteralsToModule(datapath_width, 4, 32);
}