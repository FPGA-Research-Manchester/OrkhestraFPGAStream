#pragma once
#include <cstdint>
class MemoryBlockInterface {
 public:
  virtual ~MemoryBlockInterface() = default;

  virtual auto GetVirtualAddress() -> volatile uint32_t* = 0;
  virtual auto GetPhysicalAddress() -> volatile uint32_t* = 0;
};