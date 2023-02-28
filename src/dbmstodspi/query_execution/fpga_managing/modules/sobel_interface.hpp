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

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Interface class which is implemented in the #Sobel class.
 */
class SobelInterface {
 public:
  virtual ~SobelInterface() = default;
  virtual void SetStreamParams(int stream_id, int chunks_per_row, int row_count) = 0;
};

}  // namespace orkhestrafs::dbmstodspi