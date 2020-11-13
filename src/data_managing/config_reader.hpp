#pragma once
#include <string>
#include <map> 
class ConfigReader
{
 public:
  static auto ParseDataTypeSizesConfig(std::string filename)
      -> std::map<std::string, double>;
};
