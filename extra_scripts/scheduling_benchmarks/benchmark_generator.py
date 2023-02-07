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


import numpy as np
import random
from enum import Enum
import json
import sys


class State(Enum):
    filter = 1
    join = 2
    arithmetic = 3
    aggregation = 4
    finish = 5


node_counter = 0
table_counter = 0


def get_filter(current_graph, before_nodes, current_join_nodes, filter_chance,
               leave_empty_join_chance, filter_lower_bound, filter_upper_bound):
    global node_counter
    if (random.random() < filter_chance):
        table_list = check_for_dependency(current_graph, before_nodes)
        current_graph["node" + str(node_counter)] = {"operation": "Filter", "capacity": [int(value) for value in np.random.randint(
            filter_lower_bound, filter_upper_bound)], "before": before_nodes.copy(), "after": [], "tables": table_list, "satisfying_bitstreams": []}
        # Assuming len is 1
        before_nodes[0] = ["node" + str(node_counter), 0]
        node_counter += 1
        if(current_join_nodes and random.random() >= leave_empty_join_chance):
            removed_element = current_join_nodes.pop(
                np.random.randint(len(current_join_nodes)))
            current_graph[before_nodes[0][0]]["after"].append(
                removed_element)
            current_graph[removed_element]["before"].append(
                [before_nodes[0][0], 0])
            current_graph[removed_element]["tables"].append("")
            return State.finish
    return State.join


def get_join(current_graph, before_nodes, next_join_nodes, join_chance):
    global table_counter
    global node_counter
    if (random.random() < join_chance):
        table_list = check_for_dependency(current_graph, before_nodes)
        current_graph["node" + str(node_counter)] = {"operation": "Linear Sort", "capacity": [], "before": before_nodes.copy(), "after": [
            "node" + str(node_counter + 1)], "tables": table_list, "satisfying_bitstreams": []}
        node_counter += 1
        current_graph["node" + str(node_counter)] = {"operation": "Merge Sort", "capacity": [], "before": [["node" + str(
            node_counter - 1), 0]], "after": ["node" + str(node_counter + 3)], "tables": [""], "satisfying_bitstreams": []}
        node_counter += 1

        next_join_nodes.append("node" + str(node_counter))

        current_graph["node" + str(node_counter)] = {"operation": "Linear Sort", "capacity": [], "before": [
        ], "after": ["node" + str(node_counter + 1)], "tables": [], "satisfying_bitstreams": []}
        node_counter += 1
        current_graph["node" + str(node_counter)] = {"operation": "Merge Sort", "capacity": [], "before": [["node" + str(
            node_counter - 1), 0]], "after": ["node" + str(node_counter + 1)], "tables": [""], "satisfying_bitstreams": []}
        node_counter += 1
        current_graph["node" + str(node_counter)] = {"operation": "Merge Join", "capacity": [], "before": [["node" + str(
            node_counter - 3), 0], ["node" + str(node_counter - 1), 0]], "after": [], "tables": ["", ""], "satisfying_bitstreams": []}
        # Assuming len is 1
        before_nodes[0] = ["node" + str(node_counter), 0]
        node_counter += 1
        return State.filter
    return State.arithmetic


def get_arithmetic(current_graph, before_nodes,
                   arithmetic_chance, multiplier_chance):
    global node_counter
    if (random.random() < arithmetic_chance):
        table_list = check_for_dependency(current_graph, before_nodes)
        operation = str(np.random.choice(["Multiplier", "Addition"], 1, p=[
            multiplier_chance, 1 - multiplier_chance])[0])
        current_graph["node" + str(node_counter)] = {"operation": operation, "capacity": [
        ], "before": before_nodes.copy(), "after": [], "tables": table_list, "satisfying_bitstreams": []}
        # Assuming len is 1
        before_nodes[0] = ["node" + str(node_counter), 0]
        node_counter += 1
        return State.arithmetic
    return State.aggregation


def get_aggregation(current_graph, before_nodes, aggregation_chance):
    global node_counter
    if (random.random() < aggregation_chance):
        table_list = check_for_dependency(current_graph, before_nodes)
        current_graph["node" + str(node_counter)] = {"operation": "Global Sum", "capacity": [
        ], "before": before_nodes.copy(), "after": [], "tables": table_list, "satisfying_bitstreams": []}
        # Assuming len is 1
        before_nodes[0] = ["node" + str(node_counter), 0]
        node_counter += 1
    return State.finish


def check_for_dependency(current_graph, before_nodes):
    global table_counter
    table_list = []
    if (len(before_nodes) == 1 and before_nodes[0] == ["", -1]):
        table_list.append("table_" + str(table_counter))
        table_counter += 1
    else:
        table_list.append("")
        # Assuming len is 1
        current_graph[before_nodes[0][0]]["after"].append(
            "node" + str(node_counter))
    return table_list


def main(argv):

    # print("Running benchmark generator")
    c_scheduler_format = int(argv[3]) != 0

    graph_file = argv[1]
    table_file = argv[2]

    with open(argv[0], "r") as stats_file:
        last_line = stats_file.readlines()[-1]
        values = last_line.split(',')[1:]
        if len(values) < 12:
            raise ValueError("Incorrect input")

        filter_chance = float(values[0])
        filter_first_lower_bound = int(values[1])
        filter_second_lower_bount = int(values[2])
        filter_first_upper_bount = int(values[3])
        filter_second_upper_bound = int(values[4])
        filter_lower_bound = [
            filter_first_lower_bound, filter_second_lower_bount]
        filter_upper_bound = [
            filter_first_upper_bount, filter_second_upper_bound]
        leave_empty_join_chance = float(values[5])
        join_chance = float(values[6])
        arithmetic_chance = float(values[7])
        aggregation_chance = float(values[8])
        multiplier_chance = float(values[9])
        query_count = int(values[10])
        table_size_lower_bound = int(values[11])
        table_size_upper_bound = int(values[12])
        node_max_limit = int(values[13])
        node_min_limit = int(values[14])

    # Hard coded for now
    table_recod_size_lower_bound = 8
    table_recod_size_upper_bound = 24

    # filter_chance = 0.5
    # filter_first_lower_bound = 1
    # filter_second_lower_bount = 10
    # filter_first_upper_bount = 5
    # filter_second_upper_bound = 20
    # filter_lower_bound = [filter_first_lower_bound, filter_second_lower_bount]
    # filter_upper_bound = [filter_first_upper_bount, filter_second_upper_bound]
    # leave_empty_join_chance = 0.5
    # join_chance = 0.5
    # arithmetic_chance = 0.5
    # aggregation_chance = 0.5
    # multiplier_chance = 0.5
    # query_count = 3
    # table_size_lower_bound = 10
    # table_size_upper_bound = 10000
    # node_max_limit = 10
    # node_min_limit = 3
    # graph_file = "test_graph.json"
    # table_file = "test_table.json"

    current_graph = {}
    current_join_nodes = []
    while len(current_graph.keys()) < node_min_limit:
        for i in range(query_count):
            next_join_nodes = []
            before_nodes = [["", -1]]
            current_state = State.filter
            while (current_state != State.finish and len(
                    current_graph.keys()) < node_max_limit):
                # Simple classless FSM for now
                if (current_state == State.filter):
                    current_state = get_filter(
                        current_graph, before_nodes, current_join_nodes, filter_chance, leave_empty_join_chance, filter_lower_bound, filter_upper_bound)
                elif (current_state == State.join):
                    current_state = get_join(
                        current_graph, before_nodes, next_join_nodes, join_chance)
                elif (current_state == State.arithmetic):
                    current_state = get_arithmetic(
                        current_graph, before_nodes, arithmetic_chance, multiplier_chance)
                elif (current_state == State.aggregation):
                    current_state = get_aggregation(
                        current_graph, before_nodes, aggregation_chance)
            current_join_nodes.extend(next_join_nodes)
            if (before_nodes[0] != ["", -1]
                    and not current_graph[before_nodes[0][0]]["after"]):
                current_graph[before_nodes[0][0]]["after"].append("")

    # For all current join fix the table
    global table_counter
    for node in current_join_nodes:
        current_graph[node]["tables"].append("table_" + str(table_counter))
        table_counter += 1
        current_graph[node]["before"].append(["", -1])

    if not c_scheduler_format:
        with open(graph_file, 'w', encoding='utf-8') as graph_json:
            json.dump(current_graph, graph_json, ensure_ascii=False, indent=4)
    else:
        new_graph = dict()
        for node_name in current_graph.keys():
            input = []
            output = []
            previous_nodes = []
            next_nodes = []
            operation_parameters = {
                "input_stream_params": [],
                "output_stream_params": [],
                "operation_params": []}
            operation_names = {
                "Filter": "kFilter",
                "Linear Sort": "kLinearSort",
                "Merge Sort": "kMergeSort",
                "Merge Join": "kJoin",
                "Multiplier": "kMultiplication",
                "Addition": "kAddition",
                "Global Sum": "kAggregationSum"}
            for table_name in current_graph[node_name]["tables"]:
                if table_name == "":
                    input.append(None)
                else:
                    input.append(table_name)
            for previous_node in current_graph[node_name]["before"]:
                if previous_node[0] == "":
                    previous_nodes.append(None)
                else:
                    previous_nodes.append(previous_node[0])
            for after_node in current_graph[node_name]["after"]:
                if after_node == "":
                    next_nodes.append(None)
                else:
                    next_nodes.append(after_node)
                output.append(None)
            operation = operation_names[current_graph[node_name]["operation"]]
            if operation == "kFilter":
                operation_params = [
                    [0, 0, current_graph[node_name]["capacity"][1]]]
                for i in range(current_graph[node_name]["capacity"][1]):
                    operation_params.append([])
                    operation_params.append([])
                    operation_params.append([])
                    operation_params.append(
                        [current_graph[node_name]["capacity"][0]])
                operation_parameters["operation_params"] = operation_params
            new_graph[node_name] = {
                "input": input,
                "output": output,
                "operation": operation,
                "previous_nodes": previous_nodes,
                "next_nodes": next_nodes,
                "operation_parameters": operation_parameters}
        with open(graph_file, 'w', encoding='utf-8') as graph_json:
            json.dump(new_graph, graph_json, ensure_ascii=False, indent=4)

    if c_scheduler_format:
        table_data = []
        for i in range(table_counter):
            table_data.append(
                {
                    "filename": "table_" + str(i),
                    "record_count": np.random.randint(
                        table_size_lower_bound,
                        table_size_upper_bound),
                    "record_size": np.random.randint(
                        table_recod_size_lower_bound,
                        table_recod_size_upper_bound),
                    "sorted_status": []})
    else:
        table_data = {}
        for i in range(table_counter):
            table_data["table_" +
                       str(i)] = {"record_count": np.random.randint(table_size_lower_bound, table_size_upper_bound), "record_size": np.random.randint(table_recod_size_lower_bound, table_recod_size_upper_bound), "sorted_sequences": []}

    with open(table_file, 'w', encoding='utf-8') as table_json:
        json.dump(table_data, table_json, ensure_ascii=False, indent=4)

    # print("Benchmark generator - Finished")

if __name__ == '__main__':
    main(sys.argv[1:])
