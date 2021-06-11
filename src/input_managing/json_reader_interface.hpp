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
  virtual auto readDataSizes(std::string json_filename)
      -> std::map<std::string, double> = 0;
  virtual auto readReqMemorySpace(std::string json_filename)
      -> std::map<std::string, int> = 0;
  virtual auto readAcceleratorLibrary(std::string json_filename)
      -> std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                  std::string> = 0;
  virtual auto ReadInputDefinition(std::string json_filename)
      -> std::map<std::string, InputNodeParameters> = 0;
};
}  // namespace dbmstodspi::input_managing