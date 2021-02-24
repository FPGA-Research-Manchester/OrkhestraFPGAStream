#pragma once
#include <vector>
class DMACrossbarSpecifier {
 public:
  // Check modulo clashes
  static auto IsInputClashing(const std::vector<int>& record_specification) -> bool;
  // Check division clashes
  static auto IsOutputClashing(const std::vector<int>& record_specification)
      -> bool;
  // Check if the output configuration is overwriting data
  static auto IsOutputOverwritingData(const std::vector<int>& record_specification)
      -> bool;
  // If you need garbage data how do I know that I have some? These functions
  // will figure it out and will change the additional values as required.
  /*auto ResolveInputClashes(const int record_size,
                           const std::vector<int> record_specification)
      -> std::vector<int>;*/

  static void ResolveInputClashesMultiChannel(
      const int record_size, std::vector<int>& record_specification,
      const int records_per_ddr_burst, int& chunks_per_record);

  static void ResolveInputClashesSingleChannel(
      const int record_size, std::vector<int>& record_specification,
      int& records_per_ddr_burst);

  /*auto ResolveOutputClashes(const int record_size,
                            const std::vector<int> record_specification)
      -> std::vector<int>;*/

  static void ResolveOutputClashesMultiChannel(
      const int record_size, std::vector<int>& record_specification,
      const int records_per_ddr_burst, int& chunks_per_record);

  static void ResolveOutputClashesSingleChannel(
      const int record_size, const std::vector<int> record_specification,
      int& records_per_ddr_burst);

  // Figure out a way when do I need to reduce the burst size. Probably need
  // some method for it. By these methods the parameters have been figured out!
  static auto ExtendSpecificationMultiChannel(
      const int record_size, const std::vector<int> record_specification,
      const int records_per_ddr_burst, const int chunks_per_record)
      -> std::vector<int>;
  static auto ExtendSpecificationSingleChannel(
      const int record_size, const std::vector<int> record_specification,
      const int records_per_ddr_burst) -> std::vector<int>;
};