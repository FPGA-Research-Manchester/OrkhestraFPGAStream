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


def count_node_operator(query_dicts, input_graph, query_name, current_node_name):
    query_dicts[query_name]["node_names"].add(current_node_name)
    query_dicts[query_name]["operation_stats"][input_graph[current_node_name]
                                               ["operation"]] += 1
    for next_node_name, stream_index in input_graph[current_node_name]["before"]:
        if stream_index != -1:
            count_node_operator(query_dicts, input_graph,
                                query_name, next_node_name)


def main():
    with open('input_graph.json') as graph_json:
        input_graph = json.load(graph_json)

    # First count global statistics for each operation
    global_operation_counters = dict()
    end_node_names = set()
    query_dicts = dict()
    for node_name, node_parameters in input_graph.items():
        global_operation_counters[node_parameters["operation"]] = global_operation_counters.get(
            node_parameters["operation"], 0) + 1
        if len(node_parameters["after"]) == 1 and node_parameters["after"][0] == "":
            end_node_names.add(node_name)

    print(f'Global operator counts: {global_operation_counters}')

    # Since all queries end with a single graph we first find all of these finishing nodes and then start adding node names to the queries
    for query_name in end_node_names:
        query_dicts[query_name] = {"node_names": set(), "operation_stats": dict.fromkeys(
            global_operation_counters.keys(), 0)}

    # Then I can go through all of the queries and note down all of their corresponding nodes and collect query specific stats.
    for query_name in query_dicts.keys():
        count_node_operator(query_dicts, input_graph, query_name, query_name)

    print(f'Number of queries: {len(query_dicts.keys())}')
    print(f'All queries: {query_dicts}')
    print()

    for operation in global_operation_counters.keys():

        current_counters = []
        for query_name in query_dicts.keys():
            current_counters.append(
                query_dicts[query_name]["operation_stats"][operation])
        print(f'{operation}:')
        print(f'Mean: {np.mean(current_counters)}')
        print(f'Std dev: {np.std(current_counters)}')
        print(f'Min: {np.min(current_counters)}')
        print(f'Max: {np.max(current_counters)}')
        print()

    # Doing nothing with tables at the moment.
    # with open('input_tables.json') as table_json:
    #     input_tables = json.load(table_json)

    # for table_name, table_parameters in input_tables.items():
    #     print(table_name)
    #     print(table_parameters)


if __name__ == '__main__':
    main()
