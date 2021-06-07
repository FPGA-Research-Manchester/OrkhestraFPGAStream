#pragma once

#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace dbmstodspi::input_managing {
class JSONReaderInterface {
 protected:
  using InputNodeParameters = std::map<
      std::string,
      std::variant<std::string, std::vector<std::string>,
                   std::map<std::string, std::vector<std::vector<int>>>>>;

 public:
  virtual ~JSONReaderInterface() = default;
  virtual std::map<std::string, double> readDataSizes(
      std::string json_filename) = 0;
  virtual std::map<std::string, int> readReqMemorySpace(
      std::string json_filename) = 0;
  virtual std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                   std::string>
  readAcceleratorLibrary(std::string json_filename) = 0;
  virtual std::map<std::string, InputNodeParameters> ReadInputDefinition(
      std::string json_filename) = 0;
};
}  // namespace dbmstodspi::input_managing