#include "config_reader.hpp"
#include <fstream>
#include <algorithm>

auto ConfigReader::ParseDataTypeSizesConfig(std::string filename)
    -> std::map<std::string, double> {
  // std::ifstream is RAII, i.e. no need to call close
  std::ifstream cFile(filename);
  if (cFile.is_open()) {
    std::map<std::string, double> config_data;
    std::string line;
    while (getline(cFile, line)) {
      line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
      if (line[0] == '#' || line.empty()) continue;
      auto delimiterPos = line.find("=");
      auto name = line.substr(0, delimiterPos);
      auto value = line.substr(delimiterPos + 1);
      config_data.insert(
          std::pair<std::string, double>(name, std::stod(value)));
    }
    return config_data;
  } else {
    // Throw error here
  }
}
