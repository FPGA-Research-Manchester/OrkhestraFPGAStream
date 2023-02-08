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
import json


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
    lastrow = None
    for row in reader:
        lastrow = row
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
    if all(query_filter_func(rowbuffer, lastrow, query_filter_key)):
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


def write_stats(stats, stats_json):
    with open(stats_json, "w") as f:
        json.dump(stats, f)

def main(argv):
    # You start this script and get out the following:
    # Which one of the 4 do you want.
    # 1/2 - Throw away everything that did timeout!
    # 3/4 - Throw away everything that didn't timeout!
    # Print out the stuff to some json file!
    # Then make the CSV as before
    # But then also do the graphs!

    # series_names = {0: "Max no-fit + Min fit + Min runs + First pos",
    #                    1: "Max no-fit + Min fit + Min runs",
    #                    2: "Max no-fit + Min fit",
    #                    3: "Pref fit",
    #                    4: "None"}
    series_names = {0: "Don't know",
                    1: "No idea",
                    2: "Good stuff",
                    3: "Hello",
                    4: "None",
                    5: "Help",
                    6: "Woooo"}

    function_dict = {
        "Avg": lambda stats_dict, value_name: np.average(stats_dict[value_name]),
        "Std Dev": lambda stats_dict, value_name: np.std(stats_dict[value_name]),
        "Count": lambda stats_dict, value_name: len(stats_dict[value_name]),
        "Std Error": lambda stats_dict, value_name: np.std(stats_dict[value_name])/math.sqrt(len(stats_dict[value_name]))}

    if len(argv) != 5:
        print(argv)
        raise ValueError("Wrong amount of args given!")

    output_count_filename = argv[0]
    input_stats_filename = argv[1]
    output_stats_filename = argv[2]
    graph_type = int(argv[3])
    stats_json = argv[4]

    # What do we want to do?

    stats = {"legit": 0, "per query": 0, "failed": 0}

    remove_timeout = False
    remove_non_timeout = False

    if graph_type == 0:
        pass  # Don't do anything!
    elif graph_type == 1:
        remove_timeout = True
    elif graph_type == 2:
        remove_timeout = True
    elif graph_type == 3:
        remove_non_timeout = True
    elif graph_type == 4:
        remove_non_timeout = True
    else:
        raise ValueError("Wrong graph type given")

    clean_stats_file_name = clear_input_and_report_counts(input_stats_filename, stats, remove_timeout, remove_non_timeout)

    print("legit:")
    print(stats["legit"])
    print("per query:")
    print(stats["per query"])
    print("failed:")
    print(stats["failed"])

    write_stats(stats, stats_json)
    write_counts_csv(output_count_filename, clean_stats_file_name)

    if graph_type == 0:
        pass  # Don't do anything!
    elif graph_type == 1:
        # Make the CSVs first!
        # 1st: Scheduling runtime, plans considered, execution time, configuration time for each heuristic (done to completion)!
        # 5 bar bar chart - If it has a timeout - throw it away
        make_heuristic_scheduling_stats(series_names, function_dict, clean_stats_file_name, output_stats_filename)
    elif graph_type == 2:
        # 2nd: Scheduling runtime, agains a lot of different statistics you are gathering! Again if it has a timeout - throw it away! - Bin packing
        pass  # This will be purely done on the graph side as we have a continuous space on both axis
    elif graph_type == 3:
        # 3rd: With different timeouts - measure runtime
        make_timeout_scheduling_stats(series_names, function_dict, clean_stats_file_name, output_stats_filename)
    elif graph_type == 4:
        # 4th: You can also do different selectivity rates - Could be cooler if some fall faster than others?
        raise ValueError("Not implemented currently!")
    else:
        raise ValueError("Wrong graph type given")

    # Here we have 4 legacy functions: In the beginning - clean data gets created, then we do the analyzing and lastly write.

    # output_count_filename = "perf_counts.csv"
    # Find stats about runs that didn't timeout
    # check_completed_runs(series_names, function_dict, output_count_filename, input_stats_filename, output_stats_filename)

    # output_count_filename = "counts.csv"
    # Count frames written with some weird filters
    # check_all_runs(series_names, function_dict, output_count_filename, input_stats_filename, output_stats_filename)

    # output_count_filename = "timeout_counts.csv"
    # Check all runs that timed out
    # check_timeout_runs(series_names, function_dict, output_count_filename, input_stats_filename, output_stats_filename)

    # output_count_filename = "exact_counts.csv"
    # For figuring out execution times based on different selectivity and heuristics
    # check_exact_runs(series_names, function_dict, output_count_filename, input_stats_filename, output_stats_filename)


def make_timeout_scheduling_stats(series_names, function_dict, clean_stats_file_name, output_filename):
    def row_filter_func(row, filter_key): return True
    row_filter_key = ""

    def filter_func(rowbuffer, row, filter_key): return [True]
    filter_key = ""

    # We want, 1) scheduling time, 2)overall performance, 3)config, 4)streaming, 5)plans compared
    y_keys = ["config_time", "exec_time", "plan_count", "exec_and_config"]
    x_values = [0.01, 0.1, 0.2, 0.4, 0.6, 0.8, 1, 2, 3]
    x_key = 'time_limit'
    series_values = [6]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    id_key = "query_id"
    read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_filename, id_key, filter_func,
                         filter_key, row_filter_func, row_filter_key)


def make_heuristic_scheduling_stats(series_names, function_dict, clean_stats_file_name, output_filename):
    def row_filter_func(row, filter_key): return True
    row_filter_key = ""

    def filter_func(rowbuffer, row, filter_key): return [True]
    filter_key = ""

    # We want, 1) scheduling time, 2)overall performance, 3)config, 4)streaming, 5)plans compared
    y_keys = ["performance_s", "config_time", "exec_time", "plan_count", "exec_and_config"]
    x_values = [80]
    x_key = 'time_limit'
    series_values = [6,5]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    id_key = "query_id"
    read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_filename, id_key, filter_func,
                         filter_key, row_filter_func, row_filter_key)



def clear_input_and_report_counts(input_filename, stats, remove_timeout, remove_non_timeout):
    if not remove_timeout and not remove_non_timeout:
        def filter_func(rowbuffer, row, filter_key): return [True]
        filter_key = ""
    elif remove_timeout and not remove_non_timeout:
        def filter_func(rowbuffer, row, filter_key):
            return [
                buffered_row[filter_key] != 0 for buffered_row in rowbuffer]
        filter_key = "timeouts"
    elif not remove_timeout and remove_non_timeout:
        def filter_func(rowbuffer, row, filter_key):
            return [
                buffered_row[filter_key] == 0 for buffered_row in rowbuffer]
        filter_key = "timeouts"
    else:
        raise ValueError("Incorrect options given!")

    # def filter_func(rowbuffer, row, filter_key): return [
    #     buffered_row[filter_key] == row[filter_key] and row[filter_key] != 0 for buffered_row in rowbuffer]

    ignore_columns = []
    return get_rid_of_queries_with_failed_runs(input_filename, filter_func, filter_key,
                                                                ignore_columns, stats)


def create_clean_stats(orig_stats_file_name, clean_stats_file_name, rowbuffer, last_id_value, id_key, valid_key, filter_func, filter_key, ignore_colums, stats):
    with open(orig_stats_file_name, newline='') as orig_csvfile:
        reader = csv.DictReader(orig_csvfile)
        with open(clean_stats_file_name, 'w', newline='') as clean_csvfile:
            writer = csv.DictWriter(
                clean_csvfile, fieldnames=reader.fieldnames)
            writer.writeheader()
            lastrow = None
            for row in reader:
                lastrow = row
                if row[id_key] != last_id_value:
                    if rowbuffer:
                        if all(buffered_row[valid_key] for buffered_row in rowbuffer) and all(filter_func(rowbuffer, row, filter_key)) and len(rowbuffer) >= stats["per query"]:
                            for buffered_row in rowbuffer:
                                for column in ignore_colums:
                                    buffered_row[column] = -1
                                writer.writerow(buffered_row)
                            stats["legit"] += 1
                            if stats["per query"]!=0 and stats["per query"]!=len(rowbuffer):
                                # Can happen when initially wrong queries were accepted
                                raise ValueError("Half finished query results accepted!")
                            else:
                                stats["per query"] = max(len(rowbuffer),stats["per query"])
                        else:
                            stats["failed"] += 1
                    rowbuffer = []
                    last_id_value = row[id_key]
                rowbuffer.append(row)
            if all(buffered_row[valid_key] for buffered_row in rowbuffer) and all(filter_func(rowbuffer, lastrow, filter_key)) and len(rowbuffer) >= stats["per query"]:
                for buffered_row in rowbuffer:
                   for column in ignore_colums:
                       buffered_row[column] = -1
                   writer.writerow(buffered_row)
                stats["legit"] += 1
            else:
                stats["failed"] += 1





def get_rid_of_queries_with_failed_runs(filename, filter_func, filter_key, ignore_columns, stats):
    # Filtering is for additional conditions on clearing query results out
    # Ignore columns is for replacing data in ignored columns with -1
    clean_stats_file_name = "clean_" + filename
    rowbuffer = []
    last_id_value = 0
    id_key = "query_id"
    valid_key = "score"

    create_clean_stats(filename, clean_stats_file_name,
                       rowbuffer, last_id_value, id_key, valid_key, filter_func, filter_key, ignore_columns, stats)
    return clean_stats_file_name


def write_counts_csv(counts_filename, clean_stats_file_name):
    with open(clean_stats_file_name, newline='') as csvfile:
        count_stats = {"final_query_count": [],
                       "global_node_count": [],
                       "node_count_mean": [],
                       "table_mean_count": [],
                       "table_size_mean": [],
                       "filter_global_count": [],
                       "filter_mean_count": [],
                       "merge_join_global_count": [],
                       "merge_join_mean_count": []}
        reader = csv.DictReader(csvfile)
        rowbuffer = []
        last_id_value = 0
        id_key = "query_id"
        # Over complicated now but can be extended in the future
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


# 1: Input count filename, 2: input stat file, 3: output stat file, 4: extra options
if __name__ == '__main__':
    main(sys.argv[1:])


# Deprecated funcs:
# For figuring out times based on different selectivity
def check_exact_runs(series_names, function_dict, counts_filename, input_filename, output_filename):
    # orig_stats_file_name = "exact_stats_gathered.csv"
    orig_stats_file_name = input_filename
    def row_filter_func(row, filter_key): return True
    row_filter_key = ""
    def filter_func(rowbuffer, row, filter_key): return [True]
    filter_key = ""
    ignore_columns = []

    # No extra filtering, no ignoring.
    stats = {"legit": 0, "per query": 0, "failed": 0}
    clean_stats_file_name = get_rid_of_queries_with_failed_runs(orig_stats_file_name, filter_func, filter_key, ignore_columns, stats)

    # Create Multiple series scatter graph plots with x and y values
    series_values = [0, 1, 2, 3, 4]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"] # What values are extracted for Y
    id_key = "query_id"
    # Multiple keys for multiple graphs
    y_keys = ["exec_time", "config_time", "performance_s"]
    x_values = [0.1, 0.25, 0.5, 0.75]
    x_key = 'selectivity'
    output_file_name = output_filename
    # output_file_name = 'exact_selectivity.csv'
    read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key)

    write_counts_csv(counts_filename, clean_stats_file_name)


# How many frames got written
def check_all_runs(series_names, function_dict, counts_filename, input_filename, output_filename):
    orig_stats_file_name = input_filename
    # orig_stats_file_name = "stats_all_correct3_gathered.csv"
    def row_filter_func(row, filter_key): return True
    row_filter_key = ""
    def filter_func(rowbuffer, row, filter_key): return [True]
    filter_key = ""
    ignore_columns = []

    stats = {"legit": 0, "per query": 0, "failed": 0}
    clean_stats_file_name = get_rid_of_queries_with_failed_runs(orig_stats_file_name, filter_func, filter_key, ignore_columns, stats)

    series_values = [0, 1, 2, 3, 4]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    id_key = "query_id"
    y_keys = ["frames_written"]
    x_values = [0.1, 0.25, 0.5, 0.75]
    x_key = 'selectivity'
    output_file_name = output_filename
    # output_file_name = 'all_3s_selectivity.csv'
    def row_filter_func(row, filter_key): return float(row[filter_key]) == 3
    row_filter_key = "time_limit"
    # Look at runs only where the timelimit was 3
    read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key)

    # A way to split data into big and small tables
    # output_file_name = 'all_3s_selectivity_table_size.csv'
    # splitting_key = 'table_size_mean'
    # splitting_values = [100000]
    # read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
    #                      x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key, splitting_key, splitting_values)

    write_counts_csv(counts_filename, clean_stats_file_name)


def check_timeout_runs(series_names, function_dict, counts_filename, input_filename, output_filename):
    orig_stats_file_name = input_filename
    # orig_stats_file_name = "stats_all_correct3_gathered.csv"
    def row_filter_func(row, filter_key): return True
    row_filter_key = ""
    filter_key = "timeouts"
    # def filter_func(rowbuffer, row, filter_key): return [
    #     buffered_row[filter_key] == row[filter_key] and row[filter_key] != 0 for buffered_row in rowbuffer]
    def filter_func(rowbuffer, row, filter_key): return [
        buffered_row[filter_key] != 0 for buffered_row in rowbuffer]
    ignore_columns = []

    # No no timeouts allowed
    stats = {"legit": 0, "per query": 0, "failed": 0}
    clean_stats_file_name = get_rid_of_queries_with_failed_runs(orig_stats_file_name, filter_func, filter_key, ignore_columns, stats)

    # Should come from argv
    y_keys = ["timeouts", "utility", "frames_written",
              "utility_per_frames", "placed_nodes"]
    x_values = [0.01, 0.1, 0.2, 0.4, 0.6, 0.8, 1, 2, 3]
    x_key = 'time_limit'
    # series_values = [0, 1, 2, 3]
    series_values = [0, 1, 2, 3, 4]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    output_file_name = output_filename
    # output_file_name = 'timeout_time_limits.csv'
    id_key = "query_id"
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

    # Filter based on selectivity
    # y_keys = ["frames_written"]
    # x_values = [0.1, 0.25, 0.5, 0.75]
    # x_key = 'selectivity'
    # output_file_name = 'timeout_selectivity.csv'
    # def row_filter_func(row, filter_key): return float(row[filter_key]) == 3
    # row_filter_key = "time_limit"
    # read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
    #                      x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key)

    # Split tables
    # output_file_name = 'timeout_selectivity_table_size.csv'
    # splitting_key = 'table_size_mean'
    # splitting_values = [100000]
    # read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
    #                      x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key, splitting_key, splitting_values)

    write_counts_csv(counts_filename, clean_stats_file_name)


def check_completed_runs(series_names, function_dict, counts_filename, input_filename, output_filename):
    orig_stats_file_name = input_filename
    # orig_stats_file_name = "stats_new_completed_gathered.csv"
    def row_filter_func(row, filter_key): return True

    row_filter_key = ""

    def filter_func(rowbuffer, row, filter_key): return [True]

    filter_key = ""
    ignore_columns = ["time_limit"]
    stats = {"legit": 0, "per query": 0, "failed": 0}
    clean_stats_file_name = get_rid_of_queries_with_failed_runs(orig_stats_file_name, filter_func, filter_key, ignore_columns, stats)

    y_keys = ["timeouts", "utility", "frames_written",
              "utility_per_frames", "placed_nodes", "performance_s"]
    x_values = [-10]
    x_key = 'time_limit'
    series_values = [0, 1, 2, 3, 4]
    series_key = 'heuristic'
    selected_functions = ["Avg", "Std Dev", "Count", "Std Error"]
    output_file_name = output_filename
    # output_file_name = 'perf_time_limits.csv'
    id_key = "query_id"
    read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
                         x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key)

    # Filter based on selectivity
    # y_keys = ["frames_written", "utility", "utility_per_frames"]
    # x_values = [0.1, 0.25, 0.5, 0.75]
    # x_key = 'selectivity'
    # output_file_name = 'perf_selectivity.csv'
    # read_and_write_stats(clean_stats_file_name, series_names, function_dict, y_keys, x_values,
    #                      x_key, series_values, series_key, selected_functions, output_file_name, id_key, filter_func, filter_key, row_filter_func, row_filter_key)

    write_counts_csv(counts_filename, clean_stats_file_name)