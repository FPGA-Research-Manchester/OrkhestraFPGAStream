import numpy as np
from enum import Enum
import json


class State(Enum):
    filter = 1
    join = 2
    arithmetic = 3
    aggregation = 4
    finish = 5


node_counter = 0
table_counter = 0


def random_selection(integer_limit):
    return np.random.randint(integer_limit)


def get_filter(current_graph, before_nodes):
    global node_counter
    if (random_selection(2)):
        table_list = check_for_dependency(current_graph, before_nodes)
        current_graph["node" + str(node_counter)] = {"operation": "Filter", "capacity": [random_selection(10), random_selection(
            10)], "before": before_nodes.copy(), "after": [], "tables": table_list, "satisfying_bitstreams": []}
        # Assuming len is 1
        before_nodes[0] = "node" + str(node_counter)
        node_counter += 1
    return State.join


def get_join(current_graph, before_nodes):
    global table_counter
    global node_counter
    if (random_selection(2)):
        table_list = check_for_dependency(current_graph, before_nodes)
        current_graph["node" + str(node_counter)] = {"operation": "Linear Sort", "capacity": [], "before": before_nodes.copy(), "after": [
            ["node" + str(node_counter+1), 0]], "tables": table_list, "satisfying_bitstreams": []}
        node_counter += 1
        current_graph["node" + str(node_counter)] = {"operation": "Merge Sort", "capacity": [], "before": ["node" + str(
            node_counter-1)], "after": [["node" + str(node_counter+1), 0]], "tables": [""], "satisfying_bitstreams": []}
        node_counter += 1
        table_list = []
        # Change this!
        table_list.append("table_" + str(table_counter))
        table_counter += 1
        current_graph["node" + str(node_counter)] = {"operation": "Linear Sort", "capacity": [], "before": [
            ""], "after": [["node" + str(node_counter+1), 0]], "tables": table_list, "satisfying_bitstreams": []}
        node_counter += 1
        current_graph["node" + str(node_counter)] = {"operation": "Merge Sort", "capacity": [], "before": ["node" + str(
            node_counter-1)], "after": [["node" + str(node_counter+1), 0]], "tables": [""], "satisfying_bitstreams": []}
        node_counter += 1
        current_graph["node" + str(node_counter)] = {"operation": "Merge Join", "capacity": [], "before": ["node" + str(
            node_counter-3), "node" + str(node_counter-1)], "after": [], "tables": ["", ""], "satisfying_bitstreams": []}
        # Assuming len is 1
        before_nodes[0] = "node" + str(node_counter)
        node_counter += 1
        return State.filter
    return State.arithmetic


def get_arithmetic(current_graph, before_nodes):
    global node_counter
    if (random_selection(2)):
        table_list = check_for_dependency(current_graph, before_nodes)
        operation = "Multiplier"
        if (random_selection(2)):
            operation = "Addition"
        current_graph["node" + str(node_counter)] = {"operation": operation, "capacity": [
        ], "before": before_nodes.copy(), "after": [], "tables": table_list, "satisfying_bitstreams": []}
        # Assuming len is 1
        before_nodes[0] = "node" + str(node_counter)
        node_counter += 1
        return State.arithmetic
    return State.aggregation


def get_aggregation(current_graph, before_nodes):
    global node_counter
    if (random_selection(2)):
        table_list = check_for_dependency(current_graph, before_nodes)
        current_graph["node" + str(node_counter)] = {"operation": "Global Sum", "capacity": [
        ], "before": before_nodes.copy(), "after": [], "tables": table_list, "satisfying_bitstreams": []}
        # Assuming len is 1
        before_nodes[0] = "node" + str(node_counter)
        node_counter += 1
    return State.finish


def check_for_dependency(current_graph, before_nodes):
    global table_counter
    table_list = []
    if (len(before_nodes) == 1 and before_nodes[0] == ""):
        table_list.append("table_" + str(table_counter))
        table_counter += 1
    else:
        table_list.append("")
        # Assuming len is 1
        current_graph[before_nodes[0]]["after"].append(
            ["node" + str(node_counter), 0])
    return table_list


def main():
    query_count = 10
    current_graph = {}
    for i in range(query_count):
        before_nodes = [""]
        current_state = State.filter
        while (current_state != State.finish):
            # Simple classless FSM for now
            if (current_state == State.filter):
                current_state = get_filter(current_graph, before_nodes)
            elif (current_state == State.join):
                current_state = get_join(current_graph, before_nodes)
            elif (current_state == State.arithmetic):
                current_state = get_arithmetic(current_graph, before_nodes)
            elif (current_state == State.aggregation):
                current_state = get_aggregation(current_graph, before_nodes)

    with open('input_graph.json', 'w', encoding='utf-8') as graph_json:
        json.dump(current_graph, graph_json, ensure_ascii=False, indent=4)


if __name__ == '__main__':
    main()
