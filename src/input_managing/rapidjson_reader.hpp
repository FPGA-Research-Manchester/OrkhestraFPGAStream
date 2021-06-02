#pragma once

#include <memory>

#include "json_reader_interface.hpp"
#include "rapidjson/document.h"

using rapidjson::Document;

namespace dbmstodspi::input_managing {
class RapidJSONReader : public JSONReaderInterface {
 private:
  std::unique_ptr<Document> read(std::string json_filename);

 public:
  ~RapidJSONReader() override = default;
  std::map<std::string, JSONReaderInterface::InputNodeParameters>
  readInputDefinition(std::string json_filename) override;
};
}  // namespace dbmstodspi::input_managing