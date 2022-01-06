# Copyright 2021 University of Manchester
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

import csv
import sys
import copy
import numpy as np
import math


def main(argv):
    # Time, heuristic, timeout, utility, frames, utility_per_frames
    # Avg, std_dev, count
    argv = ["stats35.csv"]
    with open(argv[0], newline='') as csvfile:
        values = {"timeouts": [], "utility": [],
                  "frames_written": [], "utility_per_frames": []}
        expected_time_limits = [0.01, 0.1, 0.5, 1, 2, 3]
        expected_heuristics = [0, 1, 2, 3]
        time_limit_stats_per_heuristic = dict()
        for i in expected_time_limits:
            time_limit_stats_per_heuristic[i] = copy.deepcopy(values)
        time_limit_stats = dict()
        for i in expected_heuristics:
            time_limit_stats[i] = copy.deepcopy(time_limit_stats_per_heuristic)
        reader = csv.DictReader(csvfile)
        for row in reader:
            if row['score']:
                for i in values:
                    time_limit_stats[int(row['heuristic'])][float(
                        row['time_limit'])][i].append(float(row[i]))
        # print(row['time_limit'], row['heuristic'], row['timeouts'],
        #       row['utility'], row['frames_written'], row['utility_per_frames'])

    with open('time_limits.csv', 'w', newline='') as csvfile:
        fieldnames = ['value_type', 'heuristic', 'time_limit',
                      'timeouts', 'utility', 'frames_written', 'utility_per_frames']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

        writer.writeheader()
        for heuristic in expected_heuristics:
            for time_limit in expected_time_limits:
                writer.writerow({'value_type': 'Avg', 'heuristic': heuristic, 'time_limit': time_limit, 'timeouts': np.average(time_limit_stats[heuristic][time_limit]['timeouts']), 'utility': np.average(
                    time_limit_stats[heuristic][time_limit]['utility']), 'frames_written': np.average(time_limit_stats[heuristic][time_limit]['frames_written']), 'utility_per_frames': np.average(time_limit_stats[heuristic][time_limit]['utility_per_frames'])})
        for heuristic in expected_heuristics:
            for time_limit in expected_time_limits:
                writer.writerow({'value_type': 'Std Dev', 'heuristic': heuristic, 'time_limit': time_limit, 'timeouts': np.std(time_limit_stats[heuristic][time_limit]['timeouts']), 'utility': np.std(
                    time_limit_stats[heuristic][time_limit]['utility']), 'frames_written': np.std(time_limit_stats[heuristic][time_limit]['frames_written']), 'utility_per_frames': np.std(time_limit_stats[heuristic][time_limit]['utility_per_frames'])})
        for heuristic in expected_heuristics:
            for time_limit in expected_time_limits:
                writer.writerow({'value_type': 'Count', 'heuristic': heuristic, 'time_limit': time_limit, 'timeouts': len(time_limit_stats[heuristic][time_limit]['timeouts']), 'utility': len(
                    time_limit_stats[heuristic][time_limit]['utility']), 'frames_written': len(time_limit_stats[heuristic][time_limit]['frames_written']), 'utility_per_frames': len(time_limit_stats[heuristic][time_limit]['utility_per_frames'])})
        for heuristic in expected_heuristics:
            for time_limit in expected_time_limits:
                writer.writerow({'value_type': 'Std Error', 'heuristic': heuristic, 'time_limit': time_limit, 'timeouts': np.std(time_limit_stats[heuristic][time_limit]['timeouts'])/math.sqrt(len(time_limit_stats[heuristic][time_limit]['timeouts'])), 'utility': np.std(time_limit_stats[heuristic][time_limit]['utility'])/math.sqrt(len(
                    time_limit_stats[heuristic][time_limit]['utility'])), 'frames_written': np.std(time_limit_stats[heuristic][time_limit]['frames_written'])/math.sqrt(len(time_limit_stats[heuristic][time_limit]['frames_written'])), 'utility_per_frames': np.std(time_limit_stats[heuristic][time_limit]['utility_per_frames'])/math.sqrt(len(time_limit_stats[heuristic][time_limit]['utility_per_frames']))})

    # Selectivity, frames,
    # Avg, std_dev, count
    with open(argv[0], newline='') as csvfile:
        values = {"frames_written": []}
        expected_selectivity = [0.1, 0.25, 0.5, 0.75]
        expected_heuristics = [0, 1, 2, 3]
        selectivity_stats_per_heuristic = dict()
        for i in expected_selectivity:
            selectivity_stats_per_heuristic[i] = copy.deepcopy(values)
        selectivity_stats = dict()
        for i in expected_heuristics:
            selectivity_stats[i] = copy.deepcopy(
                selectivity_stats_per_heuristic)
        reader = csv.DictReader(csvfile)
        for row in reader:
            if row['score']:
                for i in values:
                    selectivity_stats[int(row['heuristic'])][float(
                        row['selectivity'])][i].append(float(row[i]))
        # print(row['selectivity'], row['frames_written'])

    with open('selectivity.csv', 'w', newline='') as csvfile:
        fieldnames = ['value_type', 'heuristic',
                      'selectivity', 'frames_written']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

        writer.writeheader()
        for heuristic in expected_heuristics:
            for slectivity in expected_selectivity:
                writer.writerow({'value_type': 'Avg', 'heuristic': heuristic, 'selectivity': slectivity,
                                'frames_written': np.average(selectivity_stats[heuristic][slectivity]['frames_written'])})
        for heuristic in expected_heuristics:
            for slectivity in expected_selectivity:
                writer.writerow({'value_type': 'Std Dev', 'heuristic': heuristic, 'selectivity': slectivity,
                                'frames_written': np.std(selectivity_stats[heuristic][slectivity]['frames_written'])})
        for heuristic in expected_heuristics:
            for slectivity in expected_selectivity:
                writer.writerow({'value_type': 'Count', 'heuristic': heuristic, 'selectivity': slectivity,
                                'frames_written': len(selectivity_stats[heuristic][slectivity]['frames_written'])})
        for heuristic in expected_heuristics:
            for slectivity in expected_selectivity:
                writer.writerow({'value_type': 'Std Error', 'heuristic': heuristic, 'selectivity': slectivity, 'frames_written': np.std(
                    selectivity_stats[heuristic][slectivity]['frames_written'])/math.sqrt(len(selectivity_stats[heuristic][slectivity]['frames_written']))})

    # Node count, query count, stream count, stream element count
    # avg, std_dev, count, min, max,
    with open(argv[0], newline='') as csvfile:
        count_stats = {"node_count_mean": [], "final_query_count": [
        ], "table_mean_count": [], "table_size_mean": []}
        reader = csv.DictReader(csvfile)
        for row in reader:
            if row['score']:
                for i in count_stats.keys():
                    count_stats[i].append(float(row[i]))
    #  print(row['node_count_mean'], row['final_query_count'],
    #        row['table_mean_count'], row['table_size_mean'])

    with open('counts.csv', 'w', newline='') as csvfile:
        fieldnames = ['value_type', 'count',
                      'avg', 'std_dev', 'min', 'max']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

        writer.writeheader()
        for i in count_stats.keys():
            writer.writerow({'value_type': i, 'count': len(count_stats[i]), 'avg': np.average(count_stats[i]),
                             'std_dev': np.std(count_stats[i]), 'min': min(count_stats[i]), 'max': max(count_stats[i])})


if __name__ == '__main__':
    main(sys.argv[1:])
