#include "Filter.hpp"
//Filter module low driver

#define MODULE_ADDRESS_BITS 20

Filter::Filter(int* volatile ctrlAXIbaseAddress, uint32_t modulePosition)
{
	controlAXIbaseAddress = ctrlAXIbaseAddress;
	filterModulePosition = modulePosition;
}

int* volatile Filter::calculateMemoryMappedAddress(int* volatile controlAXIbaseAddress, uint32_t modulePosition, uint32_t moduleInternalAddress) {
	int* volatile returnAddress = controlAXIbaseAddress;
	returnAddress += modulePosition * (1 << MODULE_ADDRESS_BITS); // calculate the main address of the target module
	returnAddress += moduleInternalAddress;
	return returnAddress;
}

void Filter::writeToModule(int* volatile controlAXIbaseAddress, // base address of AXI Control port for PR region
	uint32_t modulePosition, // Position of the module within the PR region (Starting from 1)
	uint32_t moduleInternalAddress, // Internal address of the memory mapped register of the module
	uint32_t writeData		// Data to be written to module's register
) {
	int* volatile registerAddress = calculateMemoryMappedAddress(controlAXIbaseAddress, modulePosition, moduleInternalAddress);
	*registerAddress = writeData;
}
uint32_t Filter::readFromModule(int* volatile controlAXIbaseAddress, // base address of AXI Control port for PR region
	uint32_t modulePosition, // Position of the module within the PR region (Starting from 1)
	uint32_t moduleInternalAddress // Internal address of the memory mapped register of the module
) {
	volatile int* registerAddress = calculateMemoryMappedAddress(controlAXIbaseAddress, modulePosition, moduleInternalAddress);
	uint32_t readData = *registerAddress;
	return readData;
}



//Selects streamID and streamID manipulations
void Filter::filterSetStreamIDs(uint32_t streamIDInput, //The streamID of the stream that gets filterred
	uint32_t streamIDValidOutput, //The streamID of valid output from filters
	uint32_t streamIDInvalidOutput) { // The streamID of invalid output from filters
	writeToModule(controlAXIbaseAddress, filterModulePosition, 0, streamIDInput + (streamIDValidOutput << 8) + (streamIDInvalidOutput << 16));
}

//Selects mode of operation
void Filter::filterSetMode(bool requestOnInvalidIfLast, //Should the module rerequest packets if outcome is invalid? (Only applicable for last module in RE chain)
	bool forwardInvalidRecordFirstChunk, //Should the module forward the first chunk of invalid records? (Only applicable for last module in RE chain)
	bool forwardFullInvalidRecords, //Should the module forward all chunks of invalid records? (Only applicable for last module in RE chain)

	bool firstModuleInResourceElasticChain, //Indicating this is the first filter module (must be true if only 1 module is used)
	bool lastModuleInResourceElasticChain) { //Indicating this is the last filter module  (must be true if only 1 module is used)
	writeToModule(controlAXIbaseAddress, filterModulePosition, 0x4, (requestOnInvalidIfLast ? 16 : 0) + (forwardInvalidRecordFirstChunk ? 8 : 0) + (forwardFullInvalidRecords ? 4 : 0) + (firstModuleInResourceElasticChain ? 2 : 0) + (lastModuleInResourceElasticChain ? 1 : 0));
}

// Every 32-bit integer on any position and any chunk ID can have a different compare type and different reference value
// Moreover, they can have more than one compare type and reference value (we enumerate them as CMP0, CMP1, CMP2 ...)
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

void Filter::filterSetCompareTypes(uint32_t chunkID, // for which chunkID are the following compare types
	uint32_t DataPosition, // which 32-bit integer are the following compare types for

	uint32_t Compare_1_Type, // The type of comparison for each of up to 4 different
	uint32_t Compare_2_Type, // compares (depending on module type, e.g. for 1-Compare module, only Compare_1_Type will be used)
	uint32_t Compare_3_Type, // Compare types are defined above (e.g. FILTER_32BIT_LESS_THAN)
	uint32_t Compare_4_Type) { // In 64-bit compares, the current 32-bit integer holds the MSBits, while (DataPosition-1) holds the Least significant bits (reference value)
	writeToModule(controlAXIbaseAddress, filterModulePosition, ((1 << 15) + (DataPosition << 2) + (chunkID << 7)), (Compare_4_Type << 12 + Compare_3_Type << 8 + Compare_2_Type << 4 + Compare_1_Type));
}

void Filter::filterSetCompareReferenceValue(uint32_t chunkID, // for which chunkID is the following compare reference value
	uint32_t DataPosition, // for which 32-bit integer is the following compare reference value

	uint32_t CompareNumber, // Which CMP is this reference value for (i.e., 1, 2, 3, 4. Module with only 2 Compares per field can take CompareNumber of 1 and 2)

	uint32_t CompareReferenceValue) { // The 32-bit value we compare against. Can be anything (Can be 4 characters of text, can be a float number for equal compare etc.)
	writeToModule(controlAXIbaseAddress, filterModulePosition, ((1 << 15) + (DataPosition << 2) + (chunkID << 7) + (CompareNumber << 12)), CompareReferenceValue);
}


#define LITERAL_DONT_CARE 0
#define LITERAL_POSITIVE 1
#define LITERAL_NEGATIVE 2
struct {
	bool DNF_is_used = false;
	struct {
		struct {
			struct {
				uint8_t literalState = LITERAL_DONT_CARE; // 0 - literal is not present in the DNF clause; 1 - positive literal in the DNF clause ; 2 - negative literal in the DNF clause
			} DataPosition[32]; // Up to 32 data positions of integers (32-bit) for a maximum 1024-bit datapath (for 512-bit datapath use 0-15 only)
		} ChunkID[32]; // Every chunkID can produce different literals
	} CompareNumber[4]; // Up to four different compares with different reference values per 32-bit field (for 2 compares per field use 0-1 only)
} DNF_Clause[32]; //Up to 32 Clauses (for 16 DNF clause module use 0-15 only)

void Filter::filterSetDNFClauseLiteral(uint32_t DNF_Clause_ID /*0-31*/, uint32_t CompareNumber /*1-4*/, uint32_t ChunkID /*0-31*/, uint32_t DataPosition /*0-15 for 512-bit datapath etc*/, uint8_t LiteralType) {
	DNF_Clause[DNF_Clause_ID].DNF_is_used = true;
	DNF_Clause[DNF_Clause_ID].CompareNumber[CompareNumber].ChunkID[ChunkID].DataPosition[DataPosition].literalState = LiteralType;
}

void Filter::filterWriteDNFClauseLiteralsToModule(uint32_t DatapathWidth, uint32_t moduleComparesPerField, uint32_t moduleDNFClauses) {
	int CompareLane, DNFClause, DataPosition, ChunkID;
	for (CompareLane = 0; CompareLane < moduleComparesPerField; CompareLane++) {
		for (DataPosition = 0; DataPosition < DatapathWidth; DataPosition++) {
			for (ChunkID = 0; ChunkID < 32; ChunkID++) {
				uint32_t clausesPackedPositiveResult = 0xFFFFFFFF;
				uint32_t clausesPackedNegativeResult = 0xFFFFFFFF;
				if (DNF_Clause[DNFClause].DNF_is_used) {
					for (DNFClause = 0; DNFClause < moduleDNFClauses; DNFClause++) {
						clausesPackedPositiveResult <<= 1;
						clausesPackedNegativeResult <<= 1;
						if (DNF_Clause[DNFClause].CompareNumber[CompareLane].ChunkID[ChunkID].DataPosition[DataPosition].literalState == LITERAL_DONT_CARE) {
							clausesPackedPositiveResult |= 1;
							clausesPackedNegativeResult |= 1;
						}
						else if (DNF_Clause[DNFClause].CompareNumber[CompareLane].ChunkID[ChunkID].DataPosition[DataPosition].literalState == LITERAL_POSITIVE) {
							clausesPackedPositiveResult |= 1;
							clausesPackedNegativeResult |= 0;
						}
						else if (DNF_Clause[DNFClause].CompareNumber[CompareLane].ChunkID[ChunkID].DataPosition[DataPosition].literalState == LITERAL_NEGATIVE) {
							clausesPackedPositiveResult |= 0;
							clausesPackedNegativeResult |= 1;
						}
					}
				}
				else { // else DNF clause is unused
					clausesPackedPositiveResult = 0; // therefore it cannot satisfy
					clausesPackedNegativeResult = 0; // boolean expression
				}
				writeToModule(controlAXIbaseAddress, filterModulePosition, ((1 << 16) + (DataPosition << 2) + (1 << 7) + (ChunkID << 8) + (CompareLane << 13)), clausesPackedPositiveResult);
				writeToModule(controlAXIbaseAddress, filterModulePosition, ((1 << 16) + (DataPosition << 2) + (0 << 7) + (ChunkID << 8) + (CompareLane << 13)), clausesPackedNegativeResult);
			}
		}
	}
}
void Filter::writeDNFClauseLiteralsToFilter_1CMP_8DNF(uint32_t DatapathWidth /*1-32: 16->512bit datapath; 32->1024-bit datapath*/) {
	filterWriteDNFClauseLiteralsToModule(DatapathWidth, 1, 8);
}
void Filter::writeDNFClauseLiteralsToFilter_2CMP_16DNF(uint32_t DatapathWidth /*1-32: 16->512bit datapath; 32->1024-bit datapath*/) {
	filterWriteDNFClauseLiteralsToModule(DatapathWidth, 2, 16);
}
void Filter::writeDNFClauseLiteralsToFilter_4CMP_32DNF(uint32_t DatapathWidth /*1-32: 16->512bit datapath; 32->1024-bit datapath*/) {
	filterWriteDNFClauseLiteralsToModule(DatapathWidth, 4, 32);
}