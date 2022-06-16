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

import json
import sys
from dataclasses import dataclass

@dataclass
class Node:
    type: str
    id: int
    params: list
    inputs: list
    tables:list

def print_node(node_data):
    print(f"Node Type: {node_data['Node Type']}")
    if (node_data['Node Type'] == "Aggregate"):
        print(f"Output: {node_data['Output']}")
    if (node_data['Node Type'] == "Merge Join"):
        print(f"Merge Cond: {node_data['Merge Cond']}")
        print(f"Join Filter: {node_data['Join Filter']}")
    if (node_data['Node Type'] == "Seq Scan"):
        print(f"Filter: {node_data['Filter']}")
    if ("Plans" in node_data):
        print("[")
        for plan in node_data["Plans"]:
            print_node(plan)
        print("]")


def SetInitialNodes(node_data, all_nodes, counter, parent_counter):
    if (node_data['Node Type'] == "Sort"):
        current_node_counter = parent_counter
    else:
        current_node_counter = counter[0]
        child_to_parent_counter = current_node_counter
        counter[0] += 1
        if (node_data['Node Type'] == "Aggregate"):
            all_nodes[current_node_counter] = Node("Aggregate", current_node_counter, node_data['Output'], [], [])
        elif (node_data['Node Type'] == "Merge Join"):
            all_nodes[current_node_counter] = Node("Join", current_node_counter, [node_data['Merge Cond']], [], [])
            child_to_parent_counter = counter[0]
            counter[0] += 1
            all_nodes[child_to_parent_counter] = Node("Filter", child_to_parent_counter, [node_data['Join Filter']], [current_node_counter], [])
        elif (node_data['Node Type'] == "Seq Scan"):
            all_nodes[current_node_counter] = Node("Filter", current_node_counter, [node_data['Filter']], [], [node_data['Alias']])
        else:
            print(node_data['Node Type'])
            raise RuntimeError("Unknown node type!")
        if (parent_counter != -1):
            all_nodes[parent_counter].inputs.append(child_to_parent_counter)
    if ("Plans" in node_data):
        for plan in node_data["Plans"]:
            SetInitialNodes(plan, all_nodes, counter, current_node_counter)


def TokenizeParams(node):
    current_token = ""
    all_tokens = []
    quotes_active = False
    skipped_chars = {"\\", ":", "[", "]", "'", "{", "}", "\"", ","}
    single_chars = {"(",")","*"}
    for char in node.params[0]:
        if (quotes_active):
            if (char == "'"):
                quotes_active = False
                if (current_token):
                    all_tokens.append(current_token)
                    current_token = ""
            elif (char == "\\"):
                pass
            else:
                current_token += char
        else:
            if (char in skipped_chars or str(char).isspace()):
                if (current_token):
                    all_tokens.append(current_token)
                    current_token = ""
                if (char == "'"):
                    quotes_active = True
            elif (char in single_chars):
                if (current_token):
                    all_tokens.append(current_token)
                    current_token = ""
                all_tokens.append(char)
            else:
                current_token += char
    if (current_token):
        all_tokens.append(current_token)
    trimmed_tokens = []
    skipped_tokens = {"numeric", "bpchar"}
    removable_characters = {"\"", "{", "}"}
    for token in all_tokens:
        split_tokens = token.split('.')
        if (token in skipped_tokens):
            pass
        elif (len(split_tokens)!= 1):
            trimmed_tokens.append(split_tokens[1].upper())
        else:
            for char in removable_characters:
                token = token.replace(char,'')
            split_tokens = token.split(',')
            for new_token in split_tokens:
                trimmed_tokens.append(new_token)

    node.params = trimmed_tokens


def ParseJoinNode(all_nodes, key):
    removable_tokens = {'(','=',')'}
    new_tokens = []
    for token in all_nodes[key]:
        if (token not in removable_tokens):
            new_tokens.append(token)
    # Assuming they are in the correct order
    all_nodes[key].params = new_tokens


def ParseFilterNode(all_nodes, key):
    pass



def ParseAggregateNode(all_nodes, key, counter):
    # Get functions and literals.
    # Shunting_yard_algorithm
    operations = {"sum", "-", "+", "*", "/", "=", "!=", "<", ">", "<=", ">=", "AND", "OR", "avg", "min", "max"}
    #GetReversePolishNotation()
    pass


def main(argv):
    argv.append("q19.json")
    with open(argv[0]) as graph_json:
        input_query_graph = json.load(graph_json)

    # for independent_query in input_query_graph:
    #     print_node(independent_query["Plan"])

    all_nodes = dict()
    counter = [0]
    for independent_query in input_query_graph:
        SetInitialNodes(independent_query["Plan"], all_nodes, counter, -1)

    # Let's assume we don't explicitly have to deal with table checking.
    # Just the leaf nodes that have extra space can have the table added.
    # Deal with the tables - Clean up!


    # Then the params are a lot cleaner and then you can deal with the notation:
    for key in all_nodes.keys():
        # Remove all spaces
        # Remove all "\"
        # Make into a list of tokens!
        # If letter keep collecting until it's no longer a letter.
        #print(f"{key}:{all_nodes[key].params}")
        TokenizeParams(all_nodes[key])
        print(f"{key}:{all_nodes[key]}")
        if (all_nodes[key].type == "Aggregate"):
            ParseAggregateNode(all_nodes, key, counter)
        elif (all_nodes[key].type == "Join"):
            ParseJoinNode(all_nodes, key)
        elif (all_nodes[key].type == "Filter"):
            ParseFilterNode(all_nodes, key)
        else:
            raise RuntimeError("Incorrect type!")

    # Then you can parse the Aggregation to further nodes!
    # Then you can do the filter parsing

    # Print out the API calls
    # Put table calls inside
    # Integrate with PostgreSQL
    # Done.

    # Start thinking how to integrate with C++


if __name__ == '__main__':
    main(sys.argv[1:])
