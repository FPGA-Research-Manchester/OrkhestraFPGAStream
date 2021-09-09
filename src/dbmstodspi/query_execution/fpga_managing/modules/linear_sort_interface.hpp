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

#pragma once

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Interface class to be implemented in #LinearSort
 */
class LinearSortInterface {
 public:
  virtual ~LinearSortInterface() = default;

  virtual void SetStreamParams(int stream_id, int record_size) = 0;
  virtual void StartPrefetchingData() = 0;
};

}  // namespace orkhestrafs::dbmstodspi