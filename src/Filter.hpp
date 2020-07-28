#pragma once
#include <cstdint>

#include "AccelerationModule.hpp"
#include "FilterInterface.hpp"
class Filter : public AccelerationModule, public FilterInterface {
 private:
  void filterWriteDNFClauseLiteralsToModule(uint32_t DatapathWidth,
                                            uint32_t moduleComparesPerField,
                                            uint32_t moduleDNFClauses);

 public:
  ~Filter() override;
  Filter(int* volatile ctrlAXIbaseAddress, uint32_t modulePosition);

  void filterSetStreamIDs(uint32_t streamIDInput, uint32_t streamIDValidOutput,
                          uint32_t streamIDInvalidOutput) override;
  void filterSetMode(bool requestOnInvalidIfLast,
                     bool forwardInvalidRecordFirstChunk,
                     bool forwardFullInvalidRecords,
                     bool firstModuleInResourceElasticChain,
                     bool lastModuleInResourceElasticChain) override;
  void filterSetCompareTypes(uint32_t chunkID, uint32_t DataPosition,
                             uint32_t Compare_1_Type, uint32_t Compare_2_Type,
                             uint32_t Compare_3_Type,
                             uint32_t Compare_4_Type) override;
  void filterSetCompareReferenceValue(uint32_t chunkID, uint32_t DataPosition,
                                      uint32_t CompareNumber,
                                      uint32_t CompareReferenceValue) override;
  void filterSetDNFClauseLiteral(uint32_t DNF_Clause_ID, uint32_t CompareNumber,
                                 uint32_t ChunkID, uint32_t DataPosition,
                                 uint8_t LiteralType) override;

  void writeDNFClauseLiteralsToFilter_1CMP_8DNF(
      uint32_t DatapathWidth) override;
  void writeDNFClauseLiteralsToFilter_2CMP_16DNF(
      uint32_t DatapathWidth) override;
  void writeDNFClauseLiteralsToFilter_4CMP_32DNF(
      uint32_t DatapathWidth) override;
};
