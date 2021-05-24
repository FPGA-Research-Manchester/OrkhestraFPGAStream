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
