#pragma once
#include <cstdint>
class Filter
{
private:
	int* volatile controlAXIbaseAddress;
	uint32_t filterModulePosition;

	int* volatile calculateMemoryMappedAddress(int* volatile controlAXIbaseAddress, uint32_t modulePosition, uint32_t moduleInternalAddress);
	void writeToModule(int* volatile controlAXIbaseAddress, uint32_t modulePosition, uint32_t moduleInternalAddress, uint32_t writeData);
	uint32_t readFromModule(int* volatile controlAXIbaseAddress, uint32_t modulePosition, uint32_t moduleInternalAddress);

	void filterWriteDNFClauseLiteralsToModule(uint32_t DatapathWidth, uint32_t moduleComparesPerField, uint32_t moduleDNFClauses);
public:
	Filter(int* volatile ctrlAXIbaseAddress, uint32_t modulePosition);

	void filterSetStreamIDs(uint32_t streamIDInput, uint32_t streamIDValidOutput, uint32_t streamIDInvalidOutput);
	void filterSetMode(bool requestOnInvalidIfLast, bool forwardInvalidRecordFirstChunk, bool forwardFullInvalidRecords, bool firstModuleInResourceElasticChain, bool lastModuleInResourceElasticChain);
	void filterSetCompareTypes(uint32_t chunkID, uint32_t DataPosition, uint32_t Compare_1_Type, uint32_t Compare_2_Type, uint32_t Compare_3_Type, uint32_t Compare_4_Type);
	void filterSetCompareReferenceValue(uint32_t chunkID, uint32_t DataPosition, uint32_t CompareNumber, uint32_t CompareReferenceValue);
	void filterSetDNFClauseLiteral(uint32_t DNF_Clause_ID, uint32_t CompareNumber, uint32_t ChunkID, uint32_t DataPosition, uint8_t LiteralType);

	void writeDNFClauseLiteralsToFilter_1CMP_8DNF(uint32_t DatapathWidth);
	void writeDNFClauseLiteralsToFilter_2CMP_16DNF(uint32_t DatapathWidth);
	void writeDNFClauseLiteralsToFilter_4CMP_32DNF(uint32_t DatapathWidth);
};

