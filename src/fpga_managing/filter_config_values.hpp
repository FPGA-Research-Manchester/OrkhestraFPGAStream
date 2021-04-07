#pragma once
#include <array>
#include <utility>

/**
 * @brief Namespace for configuration data used to configure the filtering
 * module.
 */
namespace filter_config_values {
/// Enum for selecting compare function type.
enum class CompareFunctions {
  kFilter32BitLessThan = 0,
  kFilter32BitLessThanOrEqual = 1,
  kFilter32BitEqual = 2,
  kFilter32BitGreaterThanOrEqual = 3,
  kFilter32BitGreaterThan = 4,
  kFilter32BitNotEqual = 5,
  kFilter64BitLessThan = 8,
  kFilter64BitLessThanOrEqual = 9,
  kFilter64BitEqual = 10,
  kFilter64BitGreaterThanOrEqual = 11,
  kFilter64BitGreaterThan = 12,
  kFilter64BitNotEqual = 13
};

/// Enum for selecting the literal type.
enum class LiteralTypes {
  kLiteralDontCare = 0,  // The 0th one is the default one!
  kLiteralPositive = 1,
  kLiteralNegative = 2
};

/**
 * @brief Matrix type for holding DNF clause states and types.
 *
 * Which array is for what is explained in the Filter::FilterSetDNFClauseLiteral
 * function
 */
typedef std::array<
    std::pair<
        bool,
        std::array<
            std::array<std::array<filter_config_values::LiteralTypes, 32>, 32>,
            4>>,
    32>
    DNFClauseStates;
}  // namespace filter_config_values