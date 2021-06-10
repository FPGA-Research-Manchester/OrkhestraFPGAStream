#include "input_config_reader.hpp"

#include <fstream>
#include <algorithm>

using dbmstodspi::input_managing::InputConfigReader;

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
      auto value = line.substr(delimiter_position + 1);
      config_data.insert({name, value});
    }
    return config_data;
  }
  throw std::runtime_error((filename + " not found!").c_str());
}