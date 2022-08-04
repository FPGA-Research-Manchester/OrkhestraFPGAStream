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
#include "plan_evaluator_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to schedule nodes to groups of different FPGA runs.
 */
class PlanEvaluator : public PlanEvaluatorInterface {
 public:
  auto GetBestPlan(int min_run_count,
                   const std::vector<ScheduledModule>& last_configuration,
                   std::string resource_string, double utilites_scaler,
                   double config_written_scaler,
                   double utility_per_frame_scaler,
                   const std::map<std::vector<std::vector<ScheduledModule>>,
                                  ExecutionPlanSchedulingData>& plan_metadata,
                   const std::map<char, int>& cost_of_columns,
                   double streaming_speed, double configuration_speed)
      -> std::tuple<std::vector<std::vector<ScheduledModule>>,
                    std::vector<ScheduledModule>, long, long> override;

 private:
  std::map<std::string, int> cost_of_bitstreams_ = {{"binPartial_MergeSort64_7_36.bin", 2854},
                                                    {"binPartial_MergeSort64_37_66.bin", 2850},
                                                    {"binPartial_MergeSort32_46_66.bin", 2135},
                                                    {"binPartial_MergeSort32_16_36.bin", 2173},
                                                    {"binPartial_MergeSort128_49_96.bin", 3354},
                                                    {"binPartial_MergeJoin2K_67_96.bin", 2833},
                                                    {"binPartial_MergeJoin2K_37_66.bin", 2849},
                                                    {"binPartial_LinearSort512_7_36.bin", 2854},
                                                    {"binPartial_LinearSort1024_7_48.bin", 4582},
                                                    {"binPartial_Filter_37_66.bin", 2854},
                                                    {"binPartial_DecMult64b_64_84.bin", 2161},
                                                    {"binPartial_DecMult64b_4_24.bin", 2188},
                                                    {"binPartial_DecMult64b_34_54.bin", 2054},
                                                    {"binPartial_LinearSort1024_37_78.bin", 4532},
                                                    {"binPartial_ConstArith64b_55_66.bin", 1457},
                                                    {"binPartial_ConstArith64b_25_36.bin", 1435},
                                                    {"binPartial_AggregateGlobalSum_85_93.bin", 1036},
                                                    {"binPartial_AggregateGlobalSum_55_63.bin", 1030},
                                                    {"binPartial_AggregateGlobalSum_25_33.bin", 999},
                                                    {"binPartial_Filter_7_36.bin", 2858},
                                                    {"RT", 504},
                                                    {"binPartial_LinearSort512_37_66.bin", 2866},
                                                    {"binPartial_MergeSort64_67_96.bin", 2839},
                                                    {"binPartial_MergeJoin2K_7_36.bin", 2879},
                                                    {"binPartial_MergeSort128_19_66.bin", 3360},
                                                    {"binPartial_Filter_67_96.bin", 2944},
                                                    {"binPartial_LinearSort512_67_96.bin", 2842},
                                                    {"binPartial_MergeSort32_76_96.bin", 2125},
                                                    {"binPartial_ConstArith64b_85_96.bin", 1461},
  };
  static auto FindConfigWritten(
      const std::vector<std::vector<ScheduledModule>>& all_runs,
      const std::vector<ScheduledModule>& current_configuration,
      const std::map<std::string, int>& cost_of_bitstreams)
      -> std::pair<int, std::vector<ScheduledModule>>;

  static auto FindFastestPlan(
      const std::vector<long>& data_streamed,
      const std::vector<long>& configuration_time,
      double streaming_speed) -> int;

  static auto FindConfigWrittenForConfiguration(
      const std::vector<ScheduledModule>& next_config,
      const std::vector<ScheduledModule>& current_config,
      std::vector<std::string>& current_routing,
      const std::map<std::string, int>& cost_of_bitstreams)
      -> std::pair<int, std::vector<ScheduledModule>>;

  static void FindNewWrittenFrames(
      const std::vector<int>& fully_written_frames,
      std::vector<int>& written_frames,
      const std::vector<ScheduledModule>& configuration);
};

}  // namespace orkhestrafs::dbmstodspi