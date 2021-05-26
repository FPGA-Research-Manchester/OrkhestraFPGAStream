#pragma once

namespace easydspi::dbmstodspi {

class FSMRunnerInterface {
 public:
  virtual ~FSMRunnerInterface() = default;

  virtual void setFinishedFlag() = 0;
};
}  // namespace easydspi::dbmstodspi