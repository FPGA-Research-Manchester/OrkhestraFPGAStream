#pragma once
namespace filter_config_values {
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

enum class LiteralTypes {
  kLiteralDontCare = 0,
  kLiteralPositive = 1,
  kLiteralNegative = 2
};
}  // namespace filter_config_values