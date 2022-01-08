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


def write_values(writer, series_values, series_key, series_names, x_value_range, x_value_name, rowtemplate, stats, value_list, function_dict, selected_functions):
    for function_name in selected_functions:
        rowtemplate['aggregation_func'] = function_name
        for serie_value in series_values:
            rowtemplate[series_key] = series_names[serie_value]
            for x_value in x_value_range:
                rowtemplate[x_value_name] = x_value
                for value in value_list:
                    rowtemplate[value] = function_dict[function_name](
                        stats[serie_value][x_value], value)
                writer.writerow(rowtemplate)


def get_stats(csvfile, y_keys, x_values, x_key, series_values, series_key, splitting_key, splitting_values):
    y_values = dict()
    for i in y_keys:
        y_values[i] = []
    series_stats = dict()
    for i in x_values:
        series_stats[i] = copy.deepcopy(y_values)
    stats_in_range = dict()
    for i in series_values:
        stats_in_range[i] = copy.deepcopy(series_stats)
    all_stats = []
    if splitting_values:
        for i in range(len(splitting_values)):
            all_stats.append(copy.deepcopy(stats_in_range))
        read_rows_into_multiple_ranges(
            csvfile, x_key, series_key, y_values, all_stats, splitting_values, splitting_key)

    else:
        all_stats.append(copy.deepcopy(stats_in_range))
        read_rows_into_single_range(
            csvfile, x_key, series_key, y_values, all_stats)

    return all_stats


def read_rows_into_single_range(csvfile, x_key, series_key, y_values, all_stats):
    reader = csv.DictReader(csvfile)
    for row in reader:
        if row['score']:  # Not a failed scheduling hardcoded
            for i in y_values:
                all_stats[0][int(row[series_key])][float(
                    row[x_key])][i].append(float(row[i]))


def read_rows_into_multiple_ranges(csvfile, x_key, series_key, y_values, all_stats, splitting_values, splitting_key):
    reader = csv.DictReader(csvfile)
    for row in reader:
        if row['score']:
            for splitting_value_i in range(len(splitting_values)):
                if float(row[splitting_key]) <= splitting_values[splitting_value_i]:
                    for i in y_values:
                        all_stats[splitting_value_i][int(row[series_key])][float(
                            row[x_key])][i].append(float(row[i]))
                    break
                if splitting_value_i == len(splitting_values):
                    for i in y_values:
                        all_stats[splitting_value_i+1][int(row[series_key])][float(
                            row[x_key])][i].append(float(row[i]))


def read_and_write_stats(stats_file_name, series_names, function_dict, y_keys, x_values, x_key, series_values, series_key, selected_functions, output_file_name, splitting_key="", splitting_values=[]):
    with open(stats_file_name, newline='') as csvfile:
        all_stats = get_stats(csvfile, y_keys, x_values,
                              x_key, series_values, series_key, splitting_key, splitting_values)
    with open(output_file_name, 'w', newline='') as csvfile:
        write_aggregated_stats(series_names, function_dict, y_keys, x_values, x_key,
                               series_values, series_key, csvfile, all_stats, selected_functions, splitting_key, splitting_values)


def write_aggregated_stats(series_names, function_dict, y_keys, x_values, x_key, series_values, series_key, csvfile, all_stats, selected_functions, splitting_key, splitting_values):
    fieldnames = ['aggregation_func', series_key, x_key]
    if (splitting_key):
        fieldnames.append(splitting_key)
    fieldnames.extend(y_keys)
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    writer.writeheader()
    rowtemplate = dict()
    for field in fieldnames:
        rowtemplate[field] = None
    if (splitting_key):
        for splitting_value_i in range(len(splitting_values)):
            rowtemplate[splitting_key] = "<=" + \
                str(splitting_values[splitting_value_i])
            write_values(writer, series_values, series_key, series_names, x_values, x_key, rowtemplate,
                         all_stats[splitting_value_i], y_keys, function_dict, selected_functions)
        rowtemplate[splitting_key] = ">" + str(splitting_values[-1])
        write_values(writer, series_values, series_key, series_names, x_values, x_key, rowtemplate,
                     all_stats[-1], y_keys, function_dict, selected_functions)
    else:
        write_values(writer, series_values, series_key, series_names, x_values, x_key, rowtemplate,
                     all_stats[0], y_keys, function_dict, selected_functions)


def main(argv):
    series_names = {0: "Max no-fit + Min fit + Min runs + First pos",
                       1: "Max no-fit + Min fit + Min runs",
                       2: "Max no-fit + Min fit",
                       3: "Pref fit"}
    function_dict = {
        "Avg": lambda stats_dict, value_name: np.average(stats_dict[value_name]),
        "Std Dev": lambda stats_dict, value_name: np.std(stats_dict[value_name]),
        "Count": lambda stats_dict, value_name: len(stats_dict[value_name]),
        "Std Error": lambda stats_dict, value_name: np.std(stats_dict[value_name])/math.sqrt(len(stats_dict[value_name]))}

    # Time, heuristic, timeout, utility, frames, utility_per_frames
    # Avg, std_dev, count
    # Should come from argv
    y_keys = ["timeouts", "utility", "frames_written", "utility_per_frames"]
    x_values = [0.01, 0.1, 0.5, 1, 2, 3]
    x_key = 'time_limit'
    series_values = [0, 1, 2, 3]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    output_file_name = 'time_limits.csv'
    stats_file_name = "stats40.csv"
    read_and_write_stats(stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name)

    # Time, heuristic, timeout, utility, frames, utility_per_frames, table_size_mean
    # Avg, std_dev, count
    y_keys = ["timeouts", "utility", "frames_written", "utility_per_frames"]
    x_values = [0.01, 0.1, 0.5, 1, 2, 3]
    x_key = 'time_limit'
    series_values = [0, 1, 2, 3]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    output_file_name = 'time_limits_split.csv'
    stats_file_name = "stats40.csv"
    splitting_key = 'table_size_mean'
    splitting_values = [10000]
    read_and_write_stats(stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name, splitting_key, splitting_values)

    # Time, heuristic, timeout, utility, frames, utility_per_frames, table_size_mean - Equal timeouts
    # Avg, std_dev, count
    with open(stats_file_name, newline='') as csvfile:
        values = {"timeouts": [], "utility": [],
                  "frames_written": [], "utility_per_frames": []}
        value_ranges = [10000]
        expected_time_limits = [0.01, 0.1, 0.5, 1, 2, 3]
        expected_heuristics = [0, 1, 2, 3]
        time_limit_stats_per_heuristic = dict()
        for i in expected_time_limits:
            time_limit_stats_per_heuristic[i] = copy.deepcopy(values)
        time_limit_stats_higher = dict()
        for i in expected_heuristics:
            time_limit_stats_higher[i] = copy.deepcopy(
                time_limit_stats_per_heuristic)
        time_limit_stats_lower = copy.deepcopy(time_limit_stats_higher)
        reader = csv.DictReader(csvfile)
        # Collect all with equal table_size_std_dev
        equal_key = "timeouts"
        rowbuffer = []
        last_id_value = 0
        id_key = "table_size_std_dev"
        for row in reader:
            if row['score']:
                if row[id_key] != last_id_value:
                    if rowbuffer:
                        if all(buffered_row[equal_key] == row[equal_key] and row[equal_key] != 0 for buffered_row in rowbuffer):
                            for buffered_row in rowbuffer:
                                if float(buffered_row['table_size_mean']) > value_ranges[0]:
                                    for i in values:
                                        time_limit_stats_higher[int(buffered_row['heuristic'])][float(
                                            buffered_row['time_limit'])][i].append(float(buffered_row[i]))
                                else:
                                    for i in values:
                                        time_limit_stats_lower[int(buffered_row['heuristic'])][float(
                                            buffered_row['time_limit'])][i].append(float(buffered_row[i]))
                    rowbuffer = []
                    last_id_value = row[id_key]
                rowbuffer.append(row)
        # print(row['time_limit'], row['heuristic'], row['timeouts'],
        #       row['utility'], row['frames_written'], row['utility_per_frames'])

    with open('time_limits_timeouts.csv', 'w', newline='') as csvfile:
        fieldnames = ['aggregation_func', 'range', 'heuristic', 'time_limit',
                      'timeouts', 'utility', 'frames_written', 'utility_per_frames']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        rowtemplate = {
            'aggregation_func': "",
            'range': '<=' + str(value_ranges[0]),
            'heuristic': "",
            'time_limit': 0,
            'timeouts': 0,
            'utility': 0,
            'frames_written': 0,
            'utility_per_frames': 0}
        function_dict = {
            "Avg": lambda stats_dict, value_name: np.average(stats_dict[value_name]),
            "Std Dev": lambda stats_dict, value_name: np.std(stats_dict[value_name]),
            "Count": lambda stats_dict, value_name: len(stats_dict[value_name]),
            "Std Error": lambda stats_dict, value_name: np.std(stats_dict[value_name])/math.sqrt(len(stats_dict[value_name]))}
        # write_values(writer, expected_heuristics, expected_time_limits, "time_limit", rowtemplate,
        #              series_names, time_limit_stats_lower, values.keys(), function_dict)
        write_values(writer, series_values, series_key, series_names, x_values, x_key, rowtemplate,
                     time_limit_stats_lower, y_keys, function_dict, selected_functions)

        rowtemplate['range'] = '>' + str(value_ranges[0])
        # write_values(writer, expected_heuristics, expected_time_limits, "time_limit", rowtemplate,
        #              series_names, time_limit_stats_higher, values.keys(), function_dict)
        write_values(writer, series_values, series_key, series_names, x_values, x_key, rowtemplate,
                     time_limit_stats_higher, y_keys, function_dict, selected_functions)

    # Selectivity, frames,
    # Avg, std_dev, count
    y_keys = ["frames_written"]
    x_values = [0.1, 0.25, 0.5, 0.75]
    x_key = 'selectivity'
    series_values = [0, 1, 2, 3]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    output_file_name = 'selectivity.csv'
    stats_file_name = "stats40.csv"
    read_and_write_stats(stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name)

    # Selectivity, frames, table_size_mean
    # Avg, std_dev, count
    y_keys = ["frames_written"]
    x_values = [0.1, 0.25, 0.5, 0.75]
    x_key = 'selectivity'
    series_values = [0, 1, 2, 3]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    output_file_name = 'selectivity_table_size.csv'
    stats_file_name = "stats40.csv"
    splitting_key = 'table_size_mean'
    splitting_values = [10000]
    read_and_write_stats(stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name, splitting_key, splitting_values)

    # placed_nodes, time_limit
    # Avg, std_dev, count
    y_keys = ["placed_nodes"]
    x_values = [0.01, 0.1, 0.5, 1, 2, 3]
    x_key = 'time_limit'
    series_values = [0, 1, 2, 3]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    output_file_name = 'placed_nodes.csv'
    stats_file_name = "stats40.csv"
    read_and_write_stats(stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name)

    # Node count, query count, stream count, stream element count
    # avg, std_dev, count, min, max,
    with open(stats_file_name, newline='') as csvfile:
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
            writer.writerow({
                'value_type': i,
                'count': len(count_stats[i]),
                'avg': np.average(count_stats[i]),
                'std_dev': np.std(count_stats[i]),
                'min': min(count_stats[i]),
                'max': max(count_stats[i])})


if __name__ == '__main__':
    main(sys.argv[1:])
