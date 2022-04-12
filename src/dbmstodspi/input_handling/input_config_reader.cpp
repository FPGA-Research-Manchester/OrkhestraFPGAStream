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
#include "input_config_reader.hpp"

#include <algorithm>
#include <fstream>

using orkhestrafs::dbmstodspi::InputConfigReader;

auto InputConfigReader::ParseInputConfig(const std::string& filename)
    -> std::map<std::string, std::string> {
  std::ifstream filestream(filename);
  if (filestream.is_open()) {
    std::map<std::string, std::string> config_data;
    std::string line;
    while (getline(filestream, line)) {
      line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
      if (line[0] == '#' || line.empty()) {
        continue;
      }
      auto delimiter_position = line.find('=');
      auto name = line.substr(0, delimiter_position);
      std::for_each(name.begin(), name.end(),
                    [](char& c) { c = ::toupper(c); });
      auto value = line.substr(delimiter_position + 1);
      config_data.insert({name, value});
    }
    return config_data;
  }
  throw std::runtime_error((filename + " not found!").c_str());
}