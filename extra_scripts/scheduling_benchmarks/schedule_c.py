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
    # print("Running scheduler")

    subdir_name = "c_scheduling_files"

    exe_location = argv[0]
    config_location = argv[1]
    input_location = argv[2]

    stats_file = argv[3]
    table_file = argv[4]

    timeout = float(argv[5])

    with open(stats_file, "r") as file:
        for last_line in file:
            pass
    values = last_line.split(',')

    selectivity = float(values[-6])  # Not a thing yet
    selectivity = 1
    time_limit = float(values[-5])
    heuristic = int(values[-4])

    # Heuristic values:
    # 0 no heuristics
    # 1 smallest fitting or largest
    # 2 use_max_runs_cap - BnB
    # 3 First
    # 4 disallow_empty_runs - Make them as full as possible
    # 5 prio_children - Start looking with nodes that have parents scheduled
    # 6 ALL

    disallow_empty_runs = "false"
    use_max_runs_cap = "false"
    prio_children = "false"
    if heuristic == 2 or heuristic == 6:
        use_max_runs_cap = "true"
    if heuristic == 4 or heuristic == 6:
        disallow_empty_runs = "true"
    if heuristic == 5 or heuristic == 6:
        prio_children = "true"

    # Don' need these
    utility_scaler = float(values[-3])
    frame_scaler = float(values[-2])
    utility_per_frame_scaler = float(values[-1])

    config_location_full = f"{subdir_name}/{config_location}"

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
    config['top']['PRIORITISE_CHILDREN'] = str(prio_children)

    with open(config_location_full, 'w') as configfile:
        config.write(configfile)

    exe_location_full = f"./{subdir_name}/{exe_location}"

    # Now write values to config

    #print(f"{exe_location_full} -c {config_location_full} -i {input_location} -q")
    # os.system(
    # f"{exe_location_full} -c {config_location_full} -i {input_location} -q")

    # subprocess.run(
    # f"{exe_location_full} -c {config_location_full} -i {input_location} -q",
    # shell=True)

    try:
        result = subprocess.run(
            f"{exe_location_full} -c {config_location_full} -i {input_location} -q",
            check=True, shell=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        # Process is still running after 10 seconds, kill it
        result.kill()
        return

    # try:

    # except subprocess.SubprocessError:
    #     pro.kill()
    #     print("FAIL")
    #     return

    benchmark_stats_file = "benchmark_stats.json"
    with open(benchmark_stats_file) as c_stats_file:
        data = json.load(c_stats_file)
        # print(data)

    config_speed = 66
    streaming_speed = 4800

    with open(stats_file, "a") as stats_file:
        stats_file.write(",")
        stats_file.write(str(data["plan_count"]))  # Plan count
        stats_file.write(",")
        stats_file.write(str(data["discarded_placements"]))  # Placed nodes
        stats_file.write(",")
        # Discarded placements
        stats_file.write(str(data["placed_nodes"]))
        stats_file.write(",")
        stats_file.write(str(data["plans_chosen"]))  # Plans chosen
        stats_file.write(",")
        stats_file.write(str(data["run_count"]))  # run_count
        stats_file.write(",")
        stats_file.write(str(data["schedule_time"] / 1000000))  # performance_s
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
        stats_file.write(str(0))
        #stats_file.write(str((1 / data["run_count"]) /
        #                 (data["configuration_amount"] / 372)))  # utility_per_frames
        stats_file.write(",")
        stats_file.write(str(-1))  # score
        stats_file.write(",")
        stats_file.write(
            str(data["data_amount"]/1000000))  # exec_time
        stats_file.write(",")
        stats_file.write(str(
            data["configuration_amount"]/1000000))  # config_time
        stats_file.write(",")
        stats_file.write(str((
            data["configuration_amount"] / 1000000) + (data["data_amount"]/1000000)))  # overall_time

    # print("Scheduler - Finished")

if __name__ == '__main__':
    main(sys.argv[1:])
