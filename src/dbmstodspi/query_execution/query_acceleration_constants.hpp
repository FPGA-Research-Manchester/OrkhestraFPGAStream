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

namespace orkhestrafs::dbmstodspi::query_acceleration_constants {
/// How much data can fit the datapath concurrently in the same cycle.
const int kDatapathWidth = 16;
/// How many cycles of data the datapath fits.
const int kDatapathLength = 32;

/// How many integers are transfered with a DDR burst.
const int kDdrBurstSize = 512;
/// How many integers are transferred in a single clock cycle.
const int kDdrSizePerCycle = 4;

/// What is the limit for the amount of records that can be transferred.
const int kMaxRecordsPerDDRBurst = 32;

/// How much memory space is required for a single module.
const int kModuleSize = 1024 * 1024;

/// How many IO streams can there be concurrently.
const int kMaxIOStreamCount = 16;

/// Which vector contains what information for query node I/O stream params.
const struct StreamParamDefinition {
  int kStreamParamCount = 4;
  int kProjectionOffset = 0;
  int kDataTypesOffset = 1;
  int kDataSizesOffset = 2;
  int kChunkCountOffset = 3;
} kIOStreamParamDefs;
}  // namespace orkhestrafs::dbmstodspi::query_acceleration_constants