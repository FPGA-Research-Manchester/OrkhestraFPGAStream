#pragma once
#include <map>
#include <string>

namespace dbmstodspi::data_managing {

/**
 * @brief Class to read the config of available data types and their sizes.
 */
class ConfigReader {
 public:
  /**
   * @brief Method to read the data types config files which tells how big each
   * data size is.
   *
   * E.g. are the integers 32 or 64 bit.
   * @param filename Path to the config file.
   * @return Map of available data types and their size scaling values.
   */
  static auto ParseDataTypeSizesConfig(const std::string& filename)
      -> std::map<std::string, double>;
};

}  // namespace dbmstodspi::data_managing