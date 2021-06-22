#pragma once
#include <memory>
#include <vector>
#include <stdexcept>

namespace dbmstodspi::util {

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

inline auto IsValidFile(const std::string& name) -> bool {
  if (FILE* file = fopen(name.c_str(), "r")) {
    fclose(file);
    return true;
  } else {
    return false;
  }
}

}  // namespace dbmstodspi::util