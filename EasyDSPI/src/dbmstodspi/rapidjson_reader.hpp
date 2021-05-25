#pragma once

#include <memory>

#include "json_reader_interface.hpp"
#include "rapidjson/document.h"

using rapidjson::Document;

namespace easydspi::dbmstodspi {
class RapidJSONReader : public JSONReaderInterface {
 private:
  std::unique_ptr<Document> read(std::string json_filename);

 public:
  ~RapidJSONReader() override = default;
  std::map<std::string, std::string> readDriverLibrary(
      std::string json_filename) override;
  std::map<std::string, double> readDataSizes(
      std::string json_filename) override;
  std::map<std::string, int> readReqMemorySpace(
      std::string json_filename) override;
  std::map<std::vector<std::pair<std::string, std::vector<int>>>, std::string>
  readAcceleratorLibrary(std::string json_filename) override;
};
}  // namespace easydspi::dbmstodspi