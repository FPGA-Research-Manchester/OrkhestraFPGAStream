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


import json
import numpy as np
import sys


def count_node_operator(query_dicts, input_graph, query_name, current_node_name):
    current_operation = input_graph[current_node_name]["operation"]
    query_dicts[query_name]["node_names"].add(current_node_name)
    query_dicts[query_name]["operation_counts"][current_operation] += 1
    for table_name in input_graph[current_node_name]["tables"]:
        if table_name != "":
            query_dicts[query_name]["table_names"].add(table_name)
    if input_graph[current_node_name]["capacity"]:
        if current_operation in query_dicts[query_name]["operation_capacities"].keys():
            query_dicts[query_name]["operation_capacities"][current_operation].append(
                input_graph[current_node_name]["capacity"])
        else:
            query_dicts[query_name]["operation_capacities"][current_operation] = [
                input_graph[current_node_name]["capacity"]]
    for next_node_name, stream_index in input_graph[current_node_name]["before"]:
        if stream_index != -1:
            count_node_operator(query_dicts, input_graph,
                                query_name, next_node_name)


def print_stats(array_of_data, stats_list):
    #print(f'Mean: {np.mean(array_of_data)}')
    #print(f'Std dev: {np.std(array_of_data)}')
    #print(f'Min: {np.min(array_of_data)}')
    #print(f'Max: {np.max(array_of_data)}')
    #print()
    stats_list.extend([np.mean(array_of_data), np.std(
        array_of_data), np.min(array_of_data), np.max(array_of_data)])


def main(argv):
    with open(argv[1]) as graph_json:
        input_graph = json.load(graph_json)

    with open(argv[2]) as table_json:
        input_tables = json.load(table_json)

    # First count global statistics for each operation
    global_operation_counters = dict()
    end_node_names = set()
    query_dicts = dict()
    for node_name, node_parameters in input_graph.items():
        global_operation_counters[node_parameters["operation"]] = global_operation_counters.get(
            node_parameters["operation"], 0) + 1
        if len(node_parameters["after"]) == 1 and node_parameters["after"][0] == "":
            end_node_names.add(node_name)

    stats_list = []
    operator_list = ["Filter", "Linear Sort", "Merge Sort",
                     "Merge Join", "Addition", "Multiplier", "Global Sum"]

    print(f'Global operator counts: {global_operation_counters}')

    for operator in operator_list:
        stats_list.append(global_operation_counters.get(operator, 0))

    # Since all queries end with a single graph we first find all of these finishing nodes and then start adding node names to the queries
    for query_name in end_node_names:
        query_dicts[query_name] = {"node_names": set(), "operation_counts": dict.fromkeys(
            global_operation_counters.keys(), 0), "operation_capacities": dict(), "table_names": set()}

    # Then I can go through all of the queries and note down all of their corresponding nodes and collect query specific stats.
    for query_name in query_dicts.keys():
        count_node_operator(query_dicts, input_graph, query_name, query_name)

    print(f'Number of queries: {len(query_dicts.keys())}')
    print(f'All queries: {query_dicts}')
    print(f'All tables: {input_tables}')
    print()

    stats_list.append(len(query_dicts.keys()))

    for operation in operator_list:
        if operation in global_operation_counters:
            current_counters = []
            capacities = []
            for query_name in query_dicts.keys():
                current_counters.append(
                    query_dicts[query_name]["operation_counts"][operation])
                if operation in query_dicts[query_name]["operation_capacities"]:
                    for operation_instance_i in range(len(query_dicts[query_name]["operation_capacities"][operation])):
                        for capacity_parameter_i in range(len(query_dicts[query_name]["operation_capacities"][operation][operation_instance_i])):
                            if len(capacities) <= capacity_parameter_i:
                                capacities.append([query_dicts[query_name]["operation_capacities"]
                                                   [operation][operation_instance_i][capacity_parameter_i]])
                            else:
                                capacities[capacity_parameter_i].append(
                                    query_dicts[query_name]["operation_capacities"][operation][operation_instance_i][capacity_parameter_i])
            #print(f'Global {operation} count statistics:')
            print_stats(current_counters, stats_list)
            if capacities:
                for i in range(len(capacities)):
                    #print(
                    #    f'Global {operation} {i} capacity parameter statistics:')
                    print_stats(capacities[i], stats_list)
        else:
            stats_list.extend([0, 0, 0, 0])
            if operation == "Filter":
                stats_list.extend([0, 0, 0, 0, 0, 0, 0, 0])

    table_sizes = []
    for table_name, table_parameters in input_tables.items():
        table_sizes.append(table_parameters["record_count"])

    #print(f'Global table size statistics:')
    print_stats(table_sizes, stats_list)

    table_counts = []
    for query_name in query_dicts.keys():
        table_counts.append(len(query_dicts[query_name]["table_names"]))

    #print(f'Global table count statistics:')
    print_stats(table_counts, stats_list)

    converted_list = [str(element) for element in stats_list]
    with open(argv[0], "a") as stats_file:
        stats_file.write(",")
        stats_file.write(",".join(converted_list))


if __name__ == '__main__':
    main(sys.argv[1:])
