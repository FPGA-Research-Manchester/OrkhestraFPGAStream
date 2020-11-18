#pragma once
#include <string>
#include <map> 
class ConfigReader
{
 public:
  static auto ParseDataTypeSizesConfig(const std::string& filename)
      -> std::map<std::string, double>;
};
