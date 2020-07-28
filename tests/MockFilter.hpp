#pragma once
#include <cstdint>

#include "FilterInterface.hpp"
#include "gmock/gmock.h"

class MockFilter : public FilterInterface {
 public:
  MOCK_METHOD(void, FilterSetStreamIDs,
              (uint32_t streamIDInput, uint32_t streamIDValidOutput,
               uint32_t streamIDInvalidOutput),
              (override));
  MOCK_METHOD(void, FilterSetMode,
              (bool requestOnInvalidIfLast, bool forwardInvalidRecordFirstChunk,
               bool forwardFullInvalidRecords,
               bool firstModuleInResourceElasticChain,
               bool lastModuleInResourceElasticChain),
              (override));
  MOCK_METHOD(void, FilterSetCompareTypes,
              (uint32_t chunkID, uint32_t DataPosition, uint32_t Compare_1_Type,
               uint32_t Compare_2_Type, uint32_t Compare_3_Type,
               uint32_t Compare_4_Type),
              (override));
  MOCK_METHOD(void, FilterSetCompareReferenceValue,
              (uint32_t chunkID, uint32_t DataPosition, uint32_t CompareNumber,
               uint32_t CompareReferenceValue),
              (override));
  MOCK_METHOD(void, FilterSetDNFClauseLiteral,
              (uint32_t DNF_Clause_ID, uint32_t CompareNumber, uint32_t ChunkID,
               uint32_t DataPosition, uint8_t LiteralType),
              (override));

  MOCK_METHOD(void, WriteDNFClauseLiteralsToFilter_1CMP_8DNF,
              (uint32_t DatapathWidth), (override));
  MOCK_METHOD(void, WriteDNFClauseLiteralsToFilter_2CMP_16DNF,
              (uint32_t DatapathWidth), (override));
  MOCK_METHOD(void, WriteDNFClauseLiteralsToFilter_4CMP_32DNF,
              (uint32_t DatapathWidth), (override));
};