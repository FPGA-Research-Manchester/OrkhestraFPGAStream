#pragma once

#include <memory>

#include "json_reader_interface.hpp"
#include "rapidjson/document.h"

using rapidjson::Document;

namespace dbmstodspi::input_managing {
class RapidJSONReader : public JSONReaderInterface {
 private:
  static auto read(const std::string& json_filename)
      -> std::unique_ptr<Document>;
  static auto ConvertCharStringToAscii(const std::string& input_string,
                                       int output_size) -> std::vector<int>;

 public:
  ~RapidJSONReader() override = default;
  auto readDataSizes(std::string json_filename)
      -> std::map<std::string, double> override;
  auto readReqMemorySpace(std::string json_filename)
      -> std::map<std::string, int> override;
  auto readAcceleratorLibrary(std::string json_filename)
      -> std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                  std::string> override;
  auto ReadInputDefinition(std::string json_filename)
      -> std::map<std::string,
                  JSONReaderInterface::InputNodeParameters> override;
};
}  // namespace dbmstodspi::input_managing