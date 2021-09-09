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

#include "addition.hpp"

using orkhestrafs::dbmstodspi::Addition;

void Addition::DefineInput(int stream_id, int chunk_id) {
  AccelerationModule::WriteToModule(0, (chunk_id << 8) + stream_id);
}

void Addition::SetInputSigns(std::bitset<8> is_value_negative) {
  AccelerationModule::WriteToModule(4, is_value_negative.to_ulong());
}

void Addition::SetLiteralValues(
    std::array<std::pair<uint32_t, uint32_t>, 8> literal_values) {
  for (int i = 0; i < literal_values.size(); i++) {
    AccelerationModule::WriteToModule(64 + (i * 8), literal_values.at(i).first);
    AccelerationModule::WriteToModule(64 + (i * 8) + 4,
                                      literal_values.at(i).second);
  }
}
