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
  std::map<std::string, int> cost_of_bitstreams_ = {
      {"binPartial_MergeSort64_7_36.bin", 5530},
      {"binPartial_MergeSort64_37_66.bin", 5556},
      {"binPartial_MergeSort32_46_66.bin", 4188},
      {"binPartial_MergeSort32_16_36.bin", 4109},
      {"binPartial_MergeSort128_49_96.bin", 8439},
      {"binPartial_MergeJoin2K_67_96.bin", 5588},
      {"binPartial_MergeJoin2K_37_66.bin", 5621},
      {"binPartial_LinearSort512_7_36.bin", 5682},
      {"binPartial_LinearSort1024_7_48.bin", 7499},
      {"binPartial_Filter_37_66.bin", 5567},
      {"binPartial_Filter216_82_96.bin", 2802},
      {"binPartial_Filter216_52_66.bin", 2831},
      {"binPartial_DecMult64b_64_84.bin", 4047},
      {"binPartial_DecMult64b_4_24.bin", 4021},
      {"binPartial_DecMult64b_34_54.bin", 4033},
      {"binPartial_LinearSort1024_37_78.bin", 7533},
      {"binPartial_ConstArith64b_55_66.bin", 2348},
      {"binPartial_ConstArith64b_25_36.bin", 2295},
      {"binPartial_AggregateGlobalSum_85_93.bin", 1817},
      {"binPartial_AggregateGlobalSum_55_63.bin", 1810},
      {"binPartial_AggregateGlobalSum_25_33.bin", 1730},
      {"binPartial_Filter_7_36.bin", 5519},
      {"RT", 659},
      {"binPartial_LinearSort512_37_66.bin", 5708},
      {"binPartial_Filter18_25_36.bin", 2394},
      {"binPartial_Filter18_85_96.bin", 2354},
      {"binPartial_MergeSort64_67_96.bin", 5518},
      {"binPartial_MergeJoin2K_7_36.bin", 5623},
      {"binPartial_MergeSort128_19_66.bin", 8691},
      {"binPartial_Filter_67_96.bin", 5474},
      {"binPartial_LinearSort512_67_96.bin", 5657},
      {"binPartial_Filter18_55_66.bin", 2349},
      {"binPartial_MergeSort32_76_96.bin", 4107},
      {"binPartial_Filter216_22_36.bin", 2799},
      {"binPartial_ConstArith64b_85_96.bin", 2361},
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