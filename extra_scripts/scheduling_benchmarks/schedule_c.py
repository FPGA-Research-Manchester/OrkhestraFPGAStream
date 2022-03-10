# Copyright 2022 University of Manchester
#
# Licensed under the Apache License, Version 2.0(the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http:  // www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import subprocess
import sys
import os
import json
import configparser


def main(argv):
    subdir_name = "c_scheduling_files"

    exe_location = argv[0]
    config_location = argv[1]
    input_location = argv[2]

    stats_file = argv[3]
    table_file = argv[4]

    with open(stats_file, "r") as file:
        for last_line in file:
            pass
    values = last_line.split(',')

    selectivity = float(values[-6])  # Not a thing yet
    selectivity = 1
    time_limit = float(values[-5])
    heuristic = int(values[-4])

    disallow_empty_runs = "true"
    use_max_runs_cap = "true"
    if heuristic == 2 or heuristic == 3 or heuristic == 4:
        disallow_empty_runs = "false"
        use_max_runs_cap = "false"

    # Don' need these
    utility_scaler = float(values[-3])
    frame_scaler = float(values[-2])
    utility_per_frame_scaler = float(values[-1])

    config_location_full = f"{subdir_name}\\{config_location}"

    config = configparser.ConfigParser()
    with open(config_location_full) as stream:
        #config.read_string("[top]\n" + stream.read())
        config.read_string(stream.read())
    config['top']['REDUCE_RUNS'] = str(disallow_empty_runs)
    config['top']['MAX_RUNS'] = str(use_max_runs_cap)
    config['top']['HEURISTIC'] = str(heuristic)
    config['top']['TIME_LIMIT'] = str(time_limit)
    config['top']['UTILITY_SCALER'] = str(utility_scaler)
    config['top']['CONFIG_SCALER'] = str(frame_scaler)
    config['top']['UTILITY_PER_FRAME_SCALER'] = str(utility_per_frame_scaler)
    config['top']['TABLE_METADATA'] = str(table_file)

    with open(config_location_full, 'w') as configfile:
        config.write(configfile)

    exe_location_full = f".\\{subdir_name}\\{exe_location}"

    # Now write values to config

    #print(f"{exe_location_full} -c {config_location_full} -i {input_location} -q")
    # os.system(
    # f"{exe_location_full} -c {config_location_full} -i {input_location} -q")

    # subprocess.run(
    # f"{exe_location_full} -c {config_location_full} -i {input_location} -q",
    # shell=True)

    result = subprocess.run(
            f"{exe_location_full} -c {config_location_full} -i {input_location} -q",
            check=True, shell=True)

    # try:
        
    # except subprocess.SubprocessError:
    #     pro.kill()
    #     print("FAIL")
    #     return

    benchmark_stats_file = "benchmark_stats.json"
    with open(benchmark_stats_file) as c_stats_file:
        data = json.load(c_stats_file)
        # print(data)

    config_speed = 66000000
    streaming_speed = 4800000000

    with open(stats_file, "a") as stats_file:
        stats_file.write(",")
        stats_file.write(str(data["plan_count"]))  # Plan count
        stats_file.write(",")
        stats_file.write(str(data["placed_nodes"]))  # Placed nodes
        stats_file.write(",")
        # Discarded placements
        stats_file.write(str(data["discarded_placements"]))
        stats_file.write(",")
        stats_file.write(str(data["schedule_count"]))  # Plans chosen
        stats_file.write(",")
        stats_file.write(str(data["run_count"]))  # run_count
        stats_file.write(",")
        stats_file.write(str(data["overall_time"] / 1000000))  # performance_s
        stats_file.write(",")
        # performance_s
        stats_file.write(str(data["cost_eval_time"] / 1000000))
        stats_file.write(",")
        stats_file.write(str(data["timeout"]))  # timeout
        stats_file.write(",")
        stats_file.write(str(-1))  # utility
        stats_file.write(",")
        # frames_written
        stats_file.write(str(data["configuration_amount"] / 372))
        stats_file.write(",")
        stats_file.write(str((1 / data["run_count"]) /
                         (data["configuration_amount"] / 372)))  # utility_per_frames
        stats_file.write(",")
        stats_file.write(str(-1))  # score
        stats_file.write(",")
        stats_file.write(
            str(data["data_amount"] / streaming_speed))  # exec_time
        stats_file.write(",")
        stats_file.write(str(
            data["configuration_amount"] /
            config_speed))  # config_time


if __name__ == '__main__':
    main(sys.argv[1:])
