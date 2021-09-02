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

#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace easydspi::dbmstodspi {
class JSONReaderInterface {
 public:
  using InputNodeParameters = std::map<
      std::string,
      std::variant<std::string, std::vector<std::string>,
                   std::map<std::string, std::vector<std::vector<int>>>>>;
  virtual ~JSONReaderInterface() = default;
  virtual auto ReadDataSizes(std::string json_filename)
      -> std::map<std::string, double> = 0;
  virtual auto ReadReqMemorySpace(std::string json_filename)
      -> std::map<std::string, int> = 0;
  virtual auto ReadAcceleratorLibrary(std::string json_filename)
      -> std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                  std::string> = 0;
  virtual auto ReadInputDefinition(std::string json_filename)
      -> std::map<std::string, InputNodeParameters> = 0;
};
}  // namespace easydspi::dbmstodspi