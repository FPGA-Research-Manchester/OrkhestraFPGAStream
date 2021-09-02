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
#include <vector>

namespace easydspi::dbmstodspi {

/**
 * @brief Class to create an extended specification based on the original
 * specification vector.
 */
class DMACrossbarSpecifier {
 public:
  /**
   * @brief Check modulo clashes. Two column selections in the same chunk can't
   * have the same modulo.
   * @param record_specification Initial specification vector to be checked.
   * @return Boolean noting if any modulo clashes were found.
   */
  static auto IsInputClashing(const std::vector<int>& record_specification)
      -> bool;
  /**
   * @brief Check division clashes. Two column selections in the same position
   * (column) can't have the same division result.
   * @param record_specification Initial specification vector to be checked.
   * @return Boolean noting if division clashes were found.
   */
  static auto IsOutputClashing(const std::vector<int>& record_specification)
      -> bool;

  /**
   * @brief Check if the output configuration is overwriting data.
   *
   * That means it is impossible to solve the clash only with the output
   * crossbar. This happens when too much data is duplicated within a chunk.
   * @param record_specification Initial specification vector to be checked.
   * @return Boolean noting if the output specification is unfeasible.
   */
  static auto IsOutputOverwritingData(
      const std::vector<int>& record_specification) -> bool;

  /**
   * @brief Resolve multi-channel clashes.
   *
   * To make space to resolve the clashes the crossbar configuration space
   * creating variable could get changed. Yet to be implemented.
   * @param record_size How many integers worth of data is there in a record.
   * @param record_specification Initial specification vector to be fixed.
   * @param records_per_ddr_burst How many records get transferred with a DDR
   * burst.
   * @param chunks_per_record How many chunks are used for each record.
   */
  static void ResolveInputClashesMultiChannel(
      int record_size, std::vector<int>& record_specification,
      int records_per_ddr_burst, int& chunks_per_record);

  /**
   * @brief Resolve single channel input crossbar clashes.
   *
   * To make space DDR burst size can be changed with the records per DDR burst
   * parameter. Yet to be implemented.
   * @param record_size How many integers worth of data there is in a record.
   * @param record_specification Initial specification vector to be fixed.
   * @param records_per_ddr_burst How many records get transferred with a DDR
   * burst.
   */
  static void ResolveInputClashesSingleChannel(
      int record_size, std::vector<int>& record_specification,
      int& records_per_ddr_burst);

  /**
   * @brief Resolve output crossbar clashes.
   *
   * Output crossbar can only deal with single channel streams. To make space
   * records per DDR burst parameter can be changed. Yet to be implemented.
   * @param record_size How many integers worth of data there is in a record.
   * @param record_specification Initial specification vector to be fixed.
   * @param records_per_ddr_burst How many records get transferred with a DDR
   * burst.
   */
  static void ResolveOutputClashesSingleChannel(
      int record_size, std::vector<int>& record_specification,
      int& records_per_ddr_burst);

  /**
   * @brief Extend multi-channel input stream specification such that the whole
   * crossbar configuration space will get configured.
   *
   * Since the first data element starts from pos 15 we want to keep the same
   * ordering on the interface and thus the extended specification is mirrored.
   *
   * @param record_size How many integers worth of data there is in a record.
   * @param record_specification Initial specification vector.
   * @param records_per_ddr_burst How many records are transferred with a DDR
   *  burst.
   * @param chunks_per_record How many chunks are used for each record.
   * @return Extended specification vector which shows which column's data goes
   *  to which location.
   */
  static auto ExtendSpecificationMultiChannel(
      int record_size, const std::vector<int>& record_specification,
      int records_per_ddr_burst, int chunks_per_record)
      -> const std::vector<int>;
  /**
   * @brief Extend single channel input stream specification such that the whole
   * crossbar configuration space will get configured.
   * @param record_size How many integers worth of data there is in a record.
   * @param record_specification Initial specification vector.
   * @param records_per_ddr_burst How many records are transferred with a DDR
   * burst.
   * @return Extended and mirrored specification vector which shows which
   * column's data goes to which location.
   */
  static auto ExtendSpecificationSingleChannel(
      int record_size, const std::vector<int>& record_specification,
      int records_per_ddr_burst) -> const std::vector<int>;

  /**
   * @brief Extend output stream specification such that the whole crossbar
   * configuration space will get configured.
   *
   * Extended output specification is for specifying what happens with the data
   * on the interface rather than how it will look like after the crossbar.
   *
   * @param record_specification Initial specification vector.
   * @param records_per_ddr_burst How many records are transferred with a DDR
   * burst.
   * @param chunks_per_record How many chunks are used for each record during
   * the burst.
   * @return Extended and mirrored specification vector which shows which
   * column's data goes to which location.
   */
  static auto ExtendOutputSpecification(
      const std::vector<int>& record_specification, int records_per_ddr_burst,
      int chunks_per_record) -> const std::vector<int>;

 private:
  static void InsertChunkIfFull(std::vector<int>& chunk_specfication,
                                std::vector<int>& extended_specification);

  static void InsertJunkDataAfterRecord(
      int start_point, int end_point, int junk_data,
      std::vector<int>& chunk_specfication,
      std::vector<int>& extended_specification);

  static void InsertValidRecordData(
      const std::vector<int>& record_specification, int start_point,
      std::vector<int>& chunk_specfication,
      std::vector<int>& extended_specification);

  static auto ExtendSpecification(const int& records_per_ddr_burst,
                                  const std::vector<int>& record_specification,
                                  const int& record_size,
                                  int post_record_junk_data,
                                  int junk_data_end_point)
      -> const std::vector<int>;

  static auto IsSpecificationValid(const std::vector<int>& record_specification)
      -> bool;
};

}  // namespace easydspi::dbmstodspi