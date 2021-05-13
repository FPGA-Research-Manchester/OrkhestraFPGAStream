#pragma once
#include <array>
#include <utility>

namespace dbmstodspi {
namespace fpga_managing {

/**
 * @brief Namespace for configuration data used to configure the filtering
 * module.
 */
namespace module_config_values {
/// Enum for selecting compare function type.
enum class FilterCompareFunctions {
  k32BitLessThan = 0,
  k32BitLessThanOrEqual = 1,
  k32BitEqual = 2,
  k32BitGreaterThanOrEqual = 3,
  k32BitGreaterThan = 4,
  k32BitNotEqual = 5,
  k64BitLessThan = 8,
  k64BitLessThanOrEqual = 9,
  k64BitEqual = 10,
  k64BitGreaterThanOrEqual = 11,
  k64BitGreaterThan = 12,
  k64BitNotEqual = 13
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
typedef std::array<std::array<std::array<LiteralTypes, 32>, 32>, 4> DNFClause;

/**
 * @brief Matrix type for holding DNF clause active states.
 */
typedef std::array<std::pair<bool, DNFClause>, 32> DNFClauseStates;
}  // namespace module_config_values

}  // namespace fpga_managing
}  // namespace dbmstodspi