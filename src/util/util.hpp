#pragma once
#include <memory>
#include <vector>

namespace dbmstodspi::util {

template <typename T>
inline auto CreateReferenceVector(
    const std::vector<std::shared_ptr<T>>& pointer_vector) -> std::vector<T> {
  std::vector<T> ref_vector;
  ref_vector.reserve(pointer_vector.size());
  for (const auto& ptr : pointer_vector) {
    ref_vector.push_back(*ptr);
  }
  return ref_vector;
}

}  // namespace dbmstodspi::util