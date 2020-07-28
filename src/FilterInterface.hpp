#pragma once
class FilterInterface {
 public:
  virtual ~FilterInterface() = default;
  ;

  virtual void filterSetStreamIDs(uint32_t streamIDInput,
                                  uint32_t streamIDValidOutput,
                                  uint32_t streamIDInvalidOutput) = 0;
  virtual void filterSetMode(bool requestOnInvalidIfLast,
                             bool forwardInvalidRecordFirstChunk,
                             bool forwardFullInvalidRecords,
                             bool firstModuleInResourceElasticChain,
                             bool lastModuleInResourceElasticChain) = 0;
  virtual void filterSetCompareTypes(uint32_t chunkID, uint32_t DataPosition,
                                     uint32_t Compare_1_Type,
                                     uint32_t Compare_2_Type,
                                     uint32_t Compare_3_Type,
                                     uint32_t Compare_4_Type) = 0;
  virtual void filterSetCompareReferenceValue(
      uint32_t chunkID, uint32_t DataPosition, uint32_t CompareNumber,
      uint32_t CompareReferenceValue) = 0;
  virtual void filterSetDNFClauseLiteral(uint32_t DNF_Clause_ID,
                                         uint32_t CompareNumber,
                                         uint32_t ChunkID,
                                         uint32_t DataPosition,
                                         uint8_t LiteralType) = 0;

  virtual void writeDNFClauseLiteralsToFilter_1CMP_8DNF(
      uint32_t DatapathWidth) = 0;
  virtual void writeDNFClauseLiteralsToFilter_2CMP_16DNF(
      uint32_t DatapathWidth) = 0;
  virtual void writeDNFClauseLiteralsToFilter_4CMP_32DNF(
      uint32_t DatapathWidth) = 0;
};