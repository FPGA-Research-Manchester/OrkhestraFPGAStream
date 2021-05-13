#pragma once

namespace dbmstodspi::fpga_managing::query_acceleration_constants {
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
}  // namespace dbmstodspi::fpga_managing::query_acceleration_constants