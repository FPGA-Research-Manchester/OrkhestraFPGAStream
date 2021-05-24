#pragma once

#include <string>

namespace easydspi::core_interfaces {
class ExecutionPlanGraphInterface {
 public:
  virtual ~ExecutionPlanGraphInterface() = default;
  // Some pointless functions
  virtual void insertData(std::string given_data) = 0;
  virtual std::string exportData() = 0;
  
};
}  // namespace easydspi::core::core_interfaces
