#pragma once

namespace easydspi::core::core_input {

enum class InputManagerValidationEnum {
  kNoValidation,
  kSchemaOnlyValidation,
  kCustomValidation,
  kSchemaAndValueValidation
};
enum class InputManagerJSONHandlerEnum { kRapidJSON, kCustom };

}  // namespace easydspi::core::core_input