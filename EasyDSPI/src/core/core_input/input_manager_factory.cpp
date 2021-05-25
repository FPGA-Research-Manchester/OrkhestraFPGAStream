#include "input_manager_factory.hpp"

#include <stdexcept>

#include "graph_creator.hpp"
#include "input_config_reader.hpp"
#include "input_manager.hpp"
#include "rapidjson_reader.hpp"

using easydspi::core::core_input::InputManager;
using easydspi::core::core_input::InputManagerFactory;
using easydspi::core_interfaces::InputManagerInterface;
using easydspi::dbmstodspi::GraphCreator;
using easydspi::dbmstodspi::InputConfigReader;
using easydspi::dbmstodspi::RapidJSONReader;

std::unique_ptr<InputManagerInterface> InputManagerFactory::getManager(
    InputManagerValidationEnum manager_type) {
  // Different options will be filled in later
  switch (manager_type) {
     case InputManagerValidationEnum::kCustomValidation:
      return std::make_unique<InputManager>(
          std::make_unique<GraphCreator>(), std::make_unique<RapidJSONReader>(),
          std::make_unique<InputConfigReader>(), nullptr, nullptr);
     case InputManagerValidationEnum::kNoValidation:
       return std::make_unique<InputManager>(
           std::make_unique<GraphCreator>(),
           std::make_unique<RapidJSONReader>(),
           std::make_unique<InputConfigReader>(), nullptr, nullptr);
     case InputManagerValidationEnum::kSchemaAndValueValidation:
       return std::make_unique<InputManager>(
           std::make_unique<GraphCreator>(),
           std::make_unique<RapidJSONReader>(),
           std::make_unique<InputConfigReader>(), nullptr, nullptr);
     case InputManagerValidationEnum::kSchemaOnlyValidation:
       return std::make_unique<InputManager>(
           std::make_unique<GraphCreator>(),
           std::make_unique<RapidJSONReader>(),
           std::make_unique<InputConfigReader>(), nullptr, nullptr);
     default:
      throw std::runtime_error(
          "Something went wrong with the input manager selection!");
  }
}