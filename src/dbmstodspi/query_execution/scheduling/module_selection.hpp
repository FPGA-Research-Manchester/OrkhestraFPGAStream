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

#include <map>
#include <string>
#include <unordered_set>
#include <utility>

#include "operation_types.hpp"
#include "scheduled_module.hpp"

using orkhestrafs::dbmstodspi::ScheduledModule;

namespace orkhestrafs::dbmstodspi {

template <class T>
inline void hash_combine(std::size_t& s, const T& v) {
  std::hash<T> h;
  s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}

template <class T>
class MyHash;

template <>
struct MyHash<ScheduledModule> {
  auto operator()(ScheduledModule const& s) const -> std::size_t {
    std::size_t res = 0;
    hash_combine(res, s.bitstream);
    hash_combine(res, s.position.first);
    hash_combine(res, s.position.second);
    hash_combine(res, s.operation_type);
    hash_combine(res, s.node_name);
    return res;
  }
};

struct PairHash {
  template <class T1, class T2>
  auto operator()(std::pair<T1, T2> const& pair) const -> std::size_t {
    std::size_t h1 = std::hash<T1>()(pair.first);
    std::size_t h2 = MyHash<T2>()(pair.second);

    return h1 ^ (h2 << 1);
  }
};

/**
 * @brief Struct to hold data about a specific module placement during
 * scheduling.
 */
class ModuleSelection {
 private:
  enum SelectionMode { kAll, kFirst, kLast, kShortest, kLongest };
  const std::map<std::string, SelectionMode> kToStringMap_ = {
      {"ALL_AVAILABLE", SelectionMode::kAll},
      {"FIRST_AVAILABLE", SelectionMode::kFirst},
      {"LAST_AVAILABLE", SelectionMode::kLast},
      {"SHORTEST_AVAILABLE", SelectionMode::kShortest},
      {"LONGEST_AVAILABLE", SelectionMode::kLongest}};

  SelectionMode value_;

  // TODO(Kaspar): Find a more beautiful way to do this as methods are not
  // allowed with enums in C.
  static auto SelectAll(
      const std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
          available_placements)
      -> std::unordered_set<std::pair<int, ScheduledModule>, PairHash>;
  static auto SelectFirst(
      const std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
          available_placements)
      -> std::unordered_set<std::pair<int, ScheduledModule>, PairHash>;
  static auto SelectLast(
      const std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
          available_placements)
      -> std::unordered_set<std::pair<int, ScheduledModule>, PairHash>;
  static auto SelectShortest(
      const std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
          available_placements)
      -> std::unordered_set<std::pair<int, ScheduledModule>, PairHash>;
  static auto SelectLongest(
      const std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
          available_placements)
      -> std::unordered_set<std::pair<int, ScheduledModule>, PairHash>;

 public:
  explicit ModuleSelection(const std::string& selection_mode)
      : value_{kToStringMap_.at(selection_mode)} {};
  [[nodiscard]] auto SelectAccordingToMode(
      const std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
          available_placements) const
      -> std::unordered_set<std::pair<int, ScheduledModule>, PairHash>;
};

}  // namespace orkhestrafs::dbmstodspi