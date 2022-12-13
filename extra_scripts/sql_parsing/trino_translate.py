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

import re
import sys
from dataclasses import dataclass


def remove_lines_and_print(filename):
    with open(filename, 'r') as f:
        lines = [line.rstrip().replace(
            'â”‚',
            '').replace(
            'â””â”€',
            '').replace('"', '') for line in f]
    indentation_dict = dict()
    blacklist = ["LocalExchange", "RemoteExchange", "Layout:", "Estimates:"]
    for line in lines[1:]:
        trimmed_line = line.lstrip()
        if (any([phrase in trimmed_line for phrase in blacklist])
                or not trimmed_line):
            continue
        current_indentation = len(line) - len(trimmed_line)
        if (current_indentation in indentation_dict):
            indentation_dict[current_indentation].append(trimmed_line)
        else:
            indentation_dict[current_indentation] = [trimmed_line]
        # print(line.replace('â”‚', ''))
    for level in indentation_dict:
        print(level)
        for line in indentation_dict[level]:
            print(line)


@dataclass
class InitialNode:
    type: str
    id: int
    params: dict
    inputs: list
    tables: list


@dataclass
class Node:
    type: str
    id: int
    params: dict
    inputs: list
    tables: list


def create_initial_nodes_dict(filename):
    with open(filename, 'r') as f:
        lines = [line.rstrip().replace('"', '')
                 for line in f if line.rstrip().replace('"', '').lstrip()][1:]

    last_indentation = -1
    last_node_id = 0
    all_initial_nodes = dict()
    parent_nodes = dict()
    blacklist = ["Layout:", "Estimates:"]
    for line in lines:
        trimmed_line = line.lstrip()
        if any([phrase in trimmed_line for phrase in blacklist]):
            continue
        if ":=" in trimmed_line:
            key, value = trimmed_line.replace('â”‚', '').split(":=")
            all_initial_nodes[last_node_id].params[key.strip()] = value.strip()
            continue
        if not all_initial_nodes:
            all_initial_nodes[last_node_id] = InitialNode(
                trimmed_line, last_node_id, dict(), [], [])
            continue
        # Need to make a new node
        if 'â””â”€' not in trimmed_line:
            raise RuntimeError(f"Unexpected line:{trimmed_line}")
        current_indentation = len(line) - len(trimmed_line)
        trimmed_line = trimmed_line.replace('â””â”€', '').lstrip()
        new_id = last_node_id + 1
        if current_indentation == last_indentation:
            all_initial_nodes[parent_nodes[last_node_id]].inputs.append(new_id)
            parent_nodes[new_id] = parent_nodes[last_node_id]
        else:
            all_initial_nodes[last_node_id].inputs.append(new_id)
            parent_nodes[new_id] = last_node_id
        all_initial_nodes[new_id] = InitialNode(
            trimmed_line, new_id, dict(), [], [])
        last_indentation = current_indentation
        last_node_id = new_id

    return all_initial_nodes


def condense_initial_nodes_dict(initial_nodes):
    if not initial_nodes:
        raise RuntimeError("No nodes!")
    for key in initial_nodes.keys():
        if not initial_nodes[key].inputs:
            initial_nodes[key].inputs.append(-1)
    keys = sorted(initial_nodes.keys())
    parent_id = keys[0]
    for key in keys:
        if not initial_nodes[key].params and len(
                initial_nodes[key].inputs) <= 1:
            for input_i in range(len(initial_nodes[parent_id].inputs)):
                if initial_nodes[parent_id].inputs[input_i] == key:
                    initial_nodes[parent_id].inputs[input_i] = initial_nodes[key].inputs[0]
            # initial_nodes[parent_id].inputs = list(map(lambda x: x.replace(
            # key, initial_nodes[key].inputs[0]),
            # initial_nodes[parent_id].inputs))

            del initial_nodes[key]
        else:
            parent_id = key

    # Next step - Take node 0 - condense the params. If doesn't contain [ - Mark for deletion
    # Go until there are no more inputs
    keys = sorted(initial_nodes.keys())
    nodes_to_delete = []
    parent_id = keys[0]
    child_param_name = ""
    for key in keys:
        if len(initial_nodes[key].inputs) > 1:
            raise RuntimeError("No support for branching plans yet!")
        if "[" not in initial_nodes[key].type:
            nodes_to_delete.append(key)
            child_param_name = initial_nodes[key].params[child_param_name].split("(")[
                1].replace(')', '')
            continue

        node_name, param_name = initial_nodes[key].type.split("[")
        param_name = param_name.replace(']', '')
        if node_name == "Output":
            if initial_nodes[key].params[param_name] == "sum":
                initial_nodes[key].type = "Aggregate"
                del initial_nodes[key].params[param_name]
                initial_nodes[key].params["function"] = "sum("
                child_param_name = "sum"
            else:
                raise RuntimeError(
                    f"Unknown node type: {initial_nodes[key].type}")
        elif node_name == "ScanFilterProject":
            if initial_nodes[key].inputs[0] != -1:
                raise RuntimeError("Unexpected middle-level filter")
            initial_nodes[key].type = "Filter"
            initial_nodes[key].params["function"] = param_name
            initial_nodes[parent_id].params["function"] = initial_nodes[parent_id].params["function"] + \
                initial_nodes[key].params[child_param_name] + ")"
            del initial_nodes[key].params[child_param_name]

    parent_id = keys[0]
    for key in keys:
        if key in nodes_to_delete:
            if len(initial_nodes[key].inputs) > 1:
                raise RuntimeError("Cannot delete node!")
            for input_i in range(len(initial_nodes[parent_id].inputs)):
                if initial_nodes[parent_id].inputs[input_i] == key:
                    initial_nodes[parent_id].inputs[input_i] = initial_nodes[key].inputs[0]
            # initial_nodes[parent_id].inputs = list(map(lambda x: x.replace(
            # key, initial_nodes[key].inputs[0]),
            # initial_nodes[parent_id].inputs))

            del initial_nodes[key]
        else:
            parent_id = key

    for id in initial_nodes:
        print(f'{id}:{initial_nodes[id]}')

    return initial_nodes


def transform_to_final_nodes_dict(initial_nodes):
    final_nodes = dict()
    for node_id in initial_nodes.keys():
        if initial_nodes[node_id].type == "Aggregate":
            final_nodes[node_id] = Node(
                initial_nodes[node_id].type, initial_nodes[node_id].id, [initial_nodes[node_id].params["function"]], initial_nodes[node_id].inputs, initial_nodes[node_id].tables)
        elif initial_nodes[node_id].type == "Filter":
            orig_params = initial_nodes[node_id].params["function"]
            # Need to get rid of the BETWEEN
            start_index_positions = [
                (match.start(), match.end()) for match in re.finditer(
                    'BETWEEN\\(', orig_params)]
            end_index_positions = []
            for start_index, end_index in start_index_positions:
                end_index_positions.append(
                    orig_params.find(
                        ")",
                        end_index,
                        len(orig_params)))
            new_params = orig_params
            for match_i in range(len(start_index_positions)):
                whole_match = orig_params[start_index_positions[match_i]
                                          [0]: end_index_positions[match_i] + 1]
                match_params = [param.strip() for param in orig_params[start_index_positions[match_i]
                                                                       [1]: end_index_positions[match_i]].split(",")]
                new_current_params = f"({match_params[0]} >= ({match_params[1]})) AND ({match_params[0]} <= ({match_params[2]}))"
                new_params = new_params.replace(
                    whole_match, new_current_params)
            # Then get the table information
            all_params = new_params.split(",")
            if "table = " not in all_params[0] or " filterPredicate = " not in all_params[1]:
                raise RuntimeError(
                    f"Unexpected params: {all_params}")

            tables = [all_params[0].replace("table = ", '')]
            filter_params = all_params[1].replace(" filterPredicate = ", '')
            # Then add quotes for DATE and VARCHAR stuff and then remove types
            current_string = ""
            new_filter_params = filter_params
            for char in filter_params:
                if char == ")":
                    if current_string == "":
                        continue
                    else:
                        current_string += ")"
                        tokens = current_string.replace(
                            "(", "").replace(")", "").split(" ")
                        if len(tokens) > 1:
                            new_token = ' '.join(tokens[1:])
                            if tokens[0] == "DATE" or "VARCHAR" in tokens[0]:
                                new_token = f'"{new_token}"'
                        else:
                            new_token = tokens[0]
                        new_filter_params = new_filter_params.replace(
                            current_string, new_token)
                        current_string = ""
                elif char == "(":
                    current_string = "("
                else:
                    current_string += char
            final_nodes[node_id] = Node(
                initial_nodes[node_id].type, initial_nodes[node_id].id, [new_filter_params], initial_nodes[node_id].inputs, tables)
        else:
            raise RuntimeError("Unsupported node type!")
    return final_nodes


def main(argv):
    # explain_input = argv[0]
    explain_input = "openLooKeng_input.txt"
    # remove_lines_and_print(explain_input)
    initial_nodes = create_initial_nodes_dict(explain_input)
    initial_nodes = condense_initial_nodes_dict(initial_nodes)
    final_nodes = transform_to_final_nodes_dict(initial_nodes)

    for key in final_nodes.keys():
        print(f"{key}:{final_nodes[key]}")
        print()


if __name__ == '__main__':
    main(sys.argv[1:])
