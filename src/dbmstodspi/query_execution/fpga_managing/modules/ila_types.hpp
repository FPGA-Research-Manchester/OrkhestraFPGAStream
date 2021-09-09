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

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Enumeration of all of the different types of data ILAs can collect.
 */
enum class ILADataTypes {
  kType = 0,
  kDataAtPos15 = 1,
  kDataAtPos14 = 2,
  kDataAtPos13 = 3,
  kDataAtPos12 = 4,
  kDataAtPos11 = 5,
  kDataAtPos10 = 6,
  kDataAtPos9 = 7,
  kDataAtPos8 = 8,
  kDataAtPos7 = 9,
  kDataAtPos6 = 10,
  kDataAtPos5 = 11,
  kDataAtPos4 = 12,
  kDataAtPos3 = 13,
  kDataAtPos2 = 14,
  kDataAtPos1 = 15,
  kDataAtPos0 = 16,
  kLast = 17,
  kStreamID = 18,
  kChunkID = 19,
  kChannelID = 20,
  kState = 21,
  kInstrType = 22,
  kInstrStreamID = 23,
  kInstrChannelID = 24,
  kInstrParam = 25,
  kJoinState = 26,
  kClockCycle = 30,
  kAxiAWV = 0,
  kAxiAWR = 1,
  kAxiAWA = 2,
  kAxiWV = 3,
  kAxiWR = 4,
  kAxiWD = 5,
  kAxiWS = 6,
  kAxiBV = 7,
  kAxiBR = 8,
  kAxiBRESP = 9,
  kAxiARV = 10,
  kAxiARR = 11,
  kAxiARA = 12,
  kAxiRV = 13,
  kAxiRR = 14,
  kAxiRD = 15,
  kAxiRRESP = 16,
  kAxiClockCycle = 17,
  kIcS = 10,
  kIcCh = 11,
  kIcI = 12,
  kIcIp = 13,
  kIcV = 14,
  kIcP = 15,
  kIcdBusy = 16,
  kIcdS = 17,
  kIcdB = 18,
  kIcdE = 19,
  kIcdCh = 20,
  kIcdBu = 21,
  kDrA = 1,
  kDrL = 2,
  kDrS = 3,
  kDrB = 4,
  kDrE = 5,
  kDrCh = 6,
  kDrBu = 7,
  kDrAr = 8,
  kDrAv = 9,
  kDrcArv = 22,
  kDrcArb = 23,
  kDrcAra = 24,
  kDrcArl = 25,
  kDrcRd = 26,
  kDrcRv = 27,
  kDrcRr = 28,
  kDrcRl = 29,
  kIbD = 30,
  kIbV = 31,
  kIbS = 32,
  kIbB = 33,
  kIbCh = 34,
  kIbCl = 35,
  kIbL = 36,
  kIbSi = 37,
  kIbVs = 38,
  kIbBu = 39,
  kUNaDin = 40,
  kUNaDout = 41,
  kUNaWadd = 42,
  kUNaRadd = 43,
  kUNaWen = 44,
  kURrDin = 45,
  kURrDout = 46,
  kURrWadd = 47,
  kURrRadd = 48,
  kURrWen = 49,
  kURpb = 50,
  kUBs = 51
};

}  // namespace dbmstodspi::fpga_managing