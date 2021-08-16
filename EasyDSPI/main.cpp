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

#include <iostream>
#include <string>
#include <vector>

#include "core.hpp"

using namespace std;
using easydspi::core::Core;

int main(int argc, char* argv[]) {
  int required_arg_count = 2 + 1;
  if (argc == 1)
    cout << "No extra command line argument passed other than program "
            "name!"
         << endl;
  else if (argc < required_arg_count) {
    cout << "Number of arguments passed is too small: " << argc << "!" << endl;
  } else if (argc > required_arg_count) {
    cout << "Number of arguments passed is too big: " << argc << "!" << endl;
  } else {
    const auto results = Core::run(argv[1], argv[2]);
    cout << "Results written to: " << endl;
    for (const auto& result : results) {
      cout << result << endl;
    }
  }
  return 0;
}
