#pragma once
namespace query_acceleration_constants {
// Size values are int data type size so x4 for bytes and x32 for bits
const int kDatapathWidth = 16; // How much data can fit the datapath concurrently in the same cycle.
const int kDatapathLength = 32; // How many cycles of data the datapath fits

const int kDdrBurstSize = 512;
const int kDdrSizePerCycle = 4; // So 1 DDR burst is 128 clock cycles.

const int kMaxRecordsPerDDRBurst = 32;
}