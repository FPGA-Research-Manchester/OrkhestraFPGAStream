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
                    if not stats[serie_value][x_value][value]:
                        raise ValueError(
                            f"No {x_value_name} values found for {series_names[serie_value]} with X={x_value}")
                    rowtemplate[value] = function_dict[function_name](
                        stats[serie_value][x_value], value)
                writer.writerow(rowtemplate)


def get_stats(csvfile, y_keys, x_values, x_key, series_values, series_key, id_key, filter_func, filter_key, row_filter_func, row_filter_key, splitting_values, splitting_key):
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
    all_stats.append(copy.deepcopy(stats_in_range))
    read_rows(
        csvfile, x_key, series_key, y_values, all_stats, id_key, filter_func, filter_key, row_filter_func, row_filter_key, splitting_values, splitting_key)

    return all_stats


def save_y_values(x_key, series_key, y_values, all_stats, row):
    for i in y_values:
        all_stats[int(row[series_key])][float(
            row[x_key])][i].append(float(row[i]))


def read_rows(csvfile, x_key, series_key, y_values, all_stats, id_key, query_filter_func, query_filter_key, row_filter_func, row_filter_key, splitting_values, splitting_key):
    reader = csv.DictReader(csvfile)
    rowbuffer = []
    last_id_value = 0
    for row in reader:
        if row[id_key] != last_id_value:
            if rowbuffer:
                if all(query_filter_func(rowbuffer, row, query_filter_key)):
                    for buffered_row in rowbuffer:
                        if splitting_values:
                            for splitting_value_i in range(len(splitting_values)):
                                if float(buffered_row[splitting_key]) <= splitting_values[splitting_value_i]:
                                    if row_filter_func(buffered_row, row_filter_key):
                                        save_y_values(x_key, series_key, y_values,
                                                      all_stats[splitting_value_i], buffered_row)
                                        break
                                if splitting_value_i == len(splitting_values) - 1:
                                    if row_filter_func(buffered_row, row_filter_key):
                                        save_y_values(x_key, series_key, y_values,
                                                      all_stats[splitting_value_i + 1], buffered_row)
                        else:
                            if row_filter_func(buffered_row, row_filter_key):
                                save_y_values(x_key, series_key, y_values,
                                              all_stats[0], buffered_row)
            rowbuffer = []
            last_id_value = row[id_key]
        rowbuffer.append(row)


def read_and_write_stats(stats_file_name, series_names, function_dict, y_keys, x_values, x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key, splitting_key="", splitting_values=[]):
    with open(stats_file_name, newline='') as csvfile:
        all_stats = get_stats(csvfile, y_keys, x_values,
                              x_key, series_values, series_key, id_key, filter_func, filter_key, row_filter_func, row_filter_key, splitting_values, splitting_key)
    with open(output_file_name, 'w', newline='') as csvfile:
        write_aggregated_stats(series_names, function_dict, y_keys, x_values, x_key,
                               series_values, series_key, csvfile, all_stats, selected_functions, splitting_key, splitting_values)


def write_aggregated_stats(series_names, function_dict, y_keys, x_values, x_key, series_values, series_key, csvfile, all_stats, selected_functions, splitting_key, splitting_values):
    fieldnames = ['aggregation_func', series_key, x_key]
    orig_split = splitting_key
    splitting_key = "Split: " + splitting_key
    fieldnames.append(splitting_key)
    fieldnames.extend(y_keys)
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    writer.writeheader()
    rowtemplate = dict()
    for field in fieldnames:
        rowtemplate[field] = None
    if (orig_split):
        for splitting_value_i in range(len(splitting_values)):
            rowtemplate[splitting_key] = "<=" + \
                str(splitting_values[splitting_value_i])
            write_values(writer, series_values, series_key, series_names, x_values, x_key, rowtemplate,
                         all_stats[splitting_value_i], y_keys, function_dict, selected_functions)
        rowtemplate[splitting_key] = ">" + str(splitting_values[-1])
        write_values(writer, series_values, series_key, series_names, x_values, x_key, rowtemplate,
                     all_stats[-1], y_keys, function_dict, selected_functions)
    else:
        rowtemplate[splitting_key] = "None"
        write_values(writer, series_values, series_key, series_names, x_values, x_key, rowtemplate,
                     all_stats[0], y_keys, function_dict, selected_functions)


def main(argv):
    series_names = {0: "Max no-fit + Min fit + Min runs + First pos",
                       1: "Max no-fit + Min fit + Min runs",
                       2: "Max no-fit + Min fit",
                       3: "Pref fit",
                       4: "None"}
    function_dict = {
        "Avg": lambda stats_dict, value_name: np.average(stats_dict[value_name]),
        "Std Dev": lambda stats_dict, value_name: np.std(stats_dict[value_name]),
        "Count": lambda stats_dict, value_name: len(stats_dict[value_name]),
        "Std Error": lambda stats_dict, value_name: np.std(stats_dict[value_name])/math.sqrt(len(stats_dict[value_name]))}

    check_completed_runs(series_names, function_dict, "perf_counts.csv")
    # check_all_runs(series_names, function_dict, "counts.csv")
    # check_timeout_runs(series_names, function_dict, "timeout_counts.csv")


def create_clean_stats(orig_stats_file_name, clean_stats_file_name, rowbuffer, last_id_value, id_key, valid_key, filter_func, filter_key):
    with open(orig_stats_file_name, newline='') as orig_csvfile:
        reader = csv.DictReader(orig_csvfile)
        with open(clean_stats_file_name, 'w', newline='') as clean_csvfile:
            writer = csv.DictWriter(
                clean_csvfile, fieldnames=reader.fieldnames)
            writer.writeheader()
            for row in reader:
                if row[id_key] != last_id_value:
                    if rowbuffer:
                        if all(buffered_row[valid_key] for buffered_row in rowbuffer) and all(filter_func(rowbuffer, row, filter_key)):
                            for buffered_row in rowbuffer:
                                buffered_row["time_limit"] = 400
                                writer.writerow(buffered_row)
                    rowbuffer = []
                    last_id_value = row[id_key]
                rowbuffer.append(row)


def check_all_runs(series_names, function_dict, counts_filename):
    orig_stats_file_name = "stats_all.csv"
    clean_stats_file_name = "clean_" + orig_stats_file_name
    rowbuffer = []
    last_id_value = 0
    id_key = "table_size_std_dev"
    valid_key = "score"
    def row_filter_func(row, filter_key): return True
    row_filter_key = ""
    def filter_func(rowbuffer, row, filter_key): return [True]
    filter_key = ""
    create_clean_stats(orig_stats_file_name, clean_stats_file_name,
                       rowbuffer, last_id_value, id_key, valid_key, filter_func, filter_key)

    series_values = [0, 1, 2, 3, 4]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    id_key = "table_size_std_dev"

    y_keys = ["frames_written"]
    x_values = [0.1, 0.25, 0.5, 0.75]
    x_key = 'selectivity'
    output_file_name = 'all_3s_selectivity.csv'
    def row_filter_func(row, filter_key): return float(row[filter_key]) == 3
    row_filter_key = "time_limit"
    read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key)

    output_file_name = 'all_3s_selectivity_table_size.csv'
    splitting_key = 'table_size_mean'
    splitting_values = [100000]
    read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key, splitting_key, splitting_values)

    write_counts_csv(counts_filename, clean_stats_file_name)


def check_timeout_runs(series_names, function_dict, counts_filename):
    orig_stats_file_name = "stats_all.csv"
    clean_stats_file_name = "timeout_clean_" + orig_stats_file_name
    rowbuffer = []
    last_id_value = 0
    id_key = "table_size_std_dev"
    valid_key = "score"
    def row_filter_func(row, filter_key): return True
    row_filter_key = ""
    filter_key = "timeouts"
    # def filter_func(rowbuffer, row, filter_key): return [
    #     buffered_row[filter_key] == row[filter_key] and row[filter_key] != 0 for buffered_row in rowbuffer]
    def filter_func(rowbuffer, row, filter_key): return [
        buffered_row[filter_key] != 0 for buffered_row in rowbuffer]
    create_clean_stats(orig_stats_file_name, clean_stats_file_name,
                       rowbuffer, last_id_value, id_key, valid_key, filter_func, filter_key)

    # Should come from argv
    y_keys = ["timeouts", "utility", "frames_written",
              "utility_per_frames", "placed_nodes"]
    x_values = [0.01, 0.1, 0.5, 1, 2, 3]
    x_key = 'time_limit'
    # series_values = [0, 1, 2, 3]
    series_values = [0, 1, 2, 3, 4]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    output_file_name = 'timeout_time_limits.csv'
    id_key = "table_size_std_dev"
    def filter_func(rowbuffer, row, filter_key): return [True]
    filter_key = ""
    read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key)

    # output_file_name = 'time_limits_split.csv'
    # splitting_key = 'table_size_mean'
    # splitting_values = [10000]
    # read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
    #                      x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key, splitting_key, splitting_values)

    # output_file_name = 'time_limits_split_with_timeouts.csv'
    # read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
    #                      x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key, splitting_key, splitting_values)

    y_keys = ["frames_written"]
    x_values = [0.1, 0.25, 0.5, 0.75]
    x_key = 'selectivity'
    output_file_name = 'timeout_selectivity.csv'
    def row_filter_func(row, filter_key): return float(row[filter_key]) == 3
    row_filter_key = "time_limit"
    read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key)

    output_file_name = 'timeout_selectivity_table_size.csv'
    splitting_key = 'table_size_mean'
    splitting_values = [100000]
    read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key, splitting_key, splitting_values)

    write_counts_csv(counts_filename, clean_stats_file_name)


def check_completed_runs(series_names, function_dict, counts_filename):
    orig_stats_file_name = "stats_perf5_thing.csv"
    clean_stats_file_name = "clean_" + orig_stats_file_name
    rowbuffer = []
    last_id_value = 0
    id_key = "table_size_std_dev"
    valid_key = "score"
    def row_filter_func(row, filter_key): return True
    row_filter_key = ""
    def filter_func(rowbuffer, row, filter_key): return [True]
    filter_key = ""
    create_clean_stats(orig_stats_file_name, clean_stats_file_name,
                       rowbuffer, last_id_value, id_key, valid_key, filter_func, filter_key)

    y_keys = ["timeouts", "utility", "frames_written",
              "utility_per_frames", "placed_nodes", "performance_s"]
    x_values = [400]
    x_key = 'time_limit'
    series_values = [0, 1, 2, 3, 4]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    output_file_name = 'perf_time_limits.csv'
    id_key = "table_size_std_dev"
    read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key)

    y_keys = ["frames_written", "utility", "utility_per_frames"]
    x_values = [0.1, 0.25, 0.5, 0.75]
    x_key = 'selectivity'
    output_file_name = 'perf_selectivity.csv'
    read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key)

    write_counts_csv(counts_filename, clean_stats_file_name)


def write_counts_csv(counts_filename, clean_stats_file_name):
    with open(clean_stats_file_name, newline='') as csvfile:
        count_stats = {"node_count_mean": [], "final_query_count": [
        ], "table_mean_count": [], "table_size_mean": [], "global_node_count": [], "filter_global_count": [], "filter_mean_count": [], "merge_join_global_count": [], "merge_join_mean_count": []}
        reader = csv.DictReader(csvfile)
        rowbuffer = []
        last_id_value = 0
        id_key = "table_size_std_dev"
        for row in reader:
            if row[id_key] != last_id_value:
                if rowbuffer:
                    for i in count_stats.keys():
                        count_stats[i].append(float(row[i]))
                rowbuffer = []
                last_id_value = row[id_key]
            rowbuffer.append(row)

    with open(counts_filename, 'w', newline='') as csvfile:
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
