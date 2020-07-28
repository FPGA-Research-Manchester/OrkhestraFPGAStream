#include "FilterSetup.hpp"

#include <cstdio>

void FilterSetup::SetupFilterModule(FilterInterface& filterModule,
                                    int inputStreamID, int outputStreamID) {
  filterModule.filterSetStreamIDs(inputStreamID, outputStreamID,
                                  outputStreamID);

  bool requestOnInvalidIfLast = true;
  bool forwardInvalidRecordFirstChunk = false;
  bool forwardFullInvalidRecords = false;

  bool firstModuleInResourceElasticChain = true;
  bool lastModuleInResourceElasticChain = true;

  filterModule.filterSetMode(
      requestOnInvalidIfLast, forwardInvalidRecordFirstChunk,
      forwardFullInvalidRecords, firstModuleInResourceElasticChain,
      lastModuleInResourceElasticChain);

  uint32_t chunkID = 1;
  uint32_t dataPosition = 1;

  uint32_t lessThanCompare = 0;
  uint32_t DONTCARECOMPARE = 0;

  filterModule.filterSetCompareTypes(chunkID, dataPosition, lessThanCompare,
                                     DONTCARECOMPARE, DONTCARECOMPARE,
                                     DONTCARECOMPARE);

  uint32_t compareNumber = 0;
  uint32_t compareReferenceValue = 12000;

  filterModule.filterSetCompareReferenceValue(
      chunkID, dataPosition, compareNumber /*+ 1*/, compareReferenceValue);

  uint32_t DNFClauseID = 0;
  uint8_t positiveLiteralType = 1;

  filterModule.filterSetDNFClauseLiteral(DNFClauseID, compareNumber, chunkID,
                                         dataPosition, positiveLiteralType);

  uint32_t datapathWidth = 16;

  filterModule.writeDNFClauseLiteralsToFilter_1CMP_8DNF(datapathWidth);
}
