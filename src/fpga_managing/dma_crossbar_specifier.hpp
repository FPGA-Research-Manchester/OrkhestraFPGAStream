#pragma once
#include <vector>
class DMACrossbarSpecifier {
 public:
  // Check modulo clashes
  static auto IsInputClashing(const std::vector<int>& record_specification)
      -> bool;
  // Check division clashes
  static auto IsOutputClashing(const std::vector<int>& record_specification)
      -> bool;
  // Check if the output configuration is overwriting data
  static auto IsOutputOverwritingData(
      const std::vector<int>& record_specification) -> bool;

  static void ResolveInputClashesMultiChannel(
      const int record_size, std::vector<int>& record_specification,
      const int records_per_ddr_burst, int& chunks_per_record);

  static void ResolveInputClashesSingleChannel(
      const int record_size, std::vector<int>& record_specification,
      int& records_per_ddr_burst);

  // Output crossbar can only deal with single channel streams
  static void ResolveOutputClashesSingleChannel(
      int record_size, std::vector<int>& record_specification,
      int& records_per_ddr_burst);

  // Since the first data element starts from pos 15 we want to keep the same
  // ordering on the interface and thus the extended specification is mirrored.
  static auto ExtendSpecificationMultiChannel(
      const int record_size, const std::vector<int>& record_specification,
      const int records_per_ddr_burst, const int chunks_per_record)
      -> const std::vector<int>;
  static auto ExtendSpecificationSingleChannel(
      const int record_size, const std::vector<int>& record_specification,
      const int records_per_ddr_burst) -> const std::vector<int>;
  // Extended output specification is for specifying what happens with the data
  // on the interface rather than how it will look like after the crossbar
  static auto ExtendOutputSpecification(
      const std::vector<int>& record_specification,
      const int records_per_ddr_burst, const int chunks_per_record)
      -> const std::vector<int>;

 private:
  static void InsertChunkIfFull(std::vector<int>& chunk_specfication,
                                std::vector<int>& extended_specification);

  static void InsertJunkDataAfterRecord(
      const int start_point, const int end_point, const int junk_data,
      std::vector<int>& chunk_specfication,
      std::vector<int>& extended_specification);

  static void InsertValidRecordData(
      const std::vector<int>& record_specification, const int start_point,
      std::vector<int>& chunk_specfication,
      std::vector<int>& extended_specification);

  static auto ExtendSpecification(const int& records_per_ddr_burst,
                                  const std::vector<int>& record_specification,
                                  const int& record_size,
                                  const int post_record_junk_data,
                                  const int junk_data_end_point)
      -> const std::vector<int>;
};