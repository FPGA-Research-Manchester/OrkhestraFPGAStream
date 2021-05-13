#pragma once
#include <array>
#include <utility>

namespace dbmstodspi::fpga_managing::module_config_values {
/// Enum for selecting compare function type.
enum class FilterCompareFunctions {
  kLessThan32Bit = 0,
  kLessThanOrEqual32Bit = 1,
  kEqual32Bit = 2,
  kGreaterThanOrEqual32Bit = 3,
  kGreaterThan32Bit = 4,
  kNotEqual32Bit = 5,
  kLessThan64Bit = 8,
  kLessThanOrEqual64Bit = 9,
  kEqual64Bit = 10,
  kGreaterThanOrEqual64Bit = 11,
  kGreaterThan64Bit = 12,
  kNotEqual64Bit = 13
};

/// Enum for selecting the literal type.
enum class LiteralTypes {
  kLiteralDontCare = 0,  // The 0th one is the default one!
  kLiteralPositive = 1,
  kLiteralNegative = 2
};

/// Enum for selecting which crossbar is being configured.
enum class DMACrossbarDirectionSelection {
  kBufferToInterfaceChunk = 0,
  kBufferToInterfacePosition = 1,
  kInterfaceToBufferChunk = 2,
  kInterfaceToBufferPosition = 3
};

/**
 * @brief DNF clauses holding literal values for different positions, chunks and
 * compare lanes.
 *
 * Which array is for what is explained in the Filter::FilterSetDNFClauseLiteral
 * function
 */
using DNFClause = std::array<std::array<std::array<LiteralTypes, 32>, 32>, 4>;

/**
 * @brief Matrix type for holding DNF clause active states.
 */
using DNFClauseStates = std::array<std::pair<bool, DNFClause>, 32>;
}  // namespace dbmstodspi::fpga_managing::module_config_values