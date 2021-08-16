/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma once

#include <memory>

#include "json_reader_interface.hpp"
#include "rapidjson/document.h"

using rapidjson::Document;

namespace easydspi::dbmstodspi {
class RapidJSONReader : public JSONReaderInterface {
  using InputNodeParameters = std::map<
      std::string,
      std::variant<std::string,
                   std::map<std::string, std::vector<std::vector<int>>>>>;

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
  std::map<std::string, InputNodeParameters> readInputDefinition(
      std::string json_filename) override;
};
}  // namespace easydspi::dbmstodspi