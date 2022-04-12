/*
Copyright 2022 University of Manchester

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

#include <exception>
#include <iostream>
#include <string>
#include <utility>

namespace orkhestrafs::dbmstodspi {
/**
 * @brief From
 * https://stackoverflow.com/questions/35215169/c-create-a-class-exception
 */
class TimeLimitException : public std::exception {
  std::string exception_message_;

 public:
  explicit TimeLimitException(std::string exception_message)
      : exception_message_(std::move(exception_message)) {}

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return exception_message_.c_str();
  }
};

}  // namespace orkhestrafs::dbmstodspi