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