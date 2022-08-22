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
#include <memory>
#include <stdexcept>
#include <vector>

/**
 * @brief Namespace for various util methods handling collections.
 */
namespace orkhestrafs::dbmstodspi::util {

/**
 * @brief Create a vector of references from a pointer vector.
 * @tparam T Type Type of the elements in the vector.
 * @param element_vector Vector of smart pointers.
 * @return Vector of references.
 */
template <typename T>
inline auto CreateReferenceVector(
    const std::vector<std::shared_ptr<T>>& pointer_vector) -> std::vector<T> {
  std::vector<T> ref_vector;
  ref_vector.reserve(pointer_vector.size());
  for (const auto& ptr : pointer_vector) {
    if (ptr) {
      ref_vector.push_back(*ptr);
    } else {
      throw std::runtime_error("Cannot convert nullptr!");
    }
  }
  return ref_vector;
}

/**
 * @brief Method to find the position index of the given element.
 * @tparam T Type Type of the elements in the vector.
 * @param element_vector Vector to search from.
 * @param element Element to search for.
 * @return Integer noting the position of the element. -1 if not found.
 */
template <typename T>
inline auto FindPositionInVector(const std::vector<T>& element_vector,
                                 const T& element) -> int {
  auto it = std::find(element_vector.begin(), element_vector.end(), element);

  if (it != element_vector.end()) {
    return it - element_vector.begin();
  } else {
    return -1;
  }
}

/**
 * @brief Method to check if the filename given is found.
 * @param name Filename.
 * @return Is file found boolean flag.
 */
inline auto IsValidFile(const std::string& name) -> bool {
  if (FILE* file = fopen(name.c_str(), "r")) {
    fclose(file);
    return true;
  }
  return false;
}

}  // namespace orkhestrafs::dbmstodspi::util