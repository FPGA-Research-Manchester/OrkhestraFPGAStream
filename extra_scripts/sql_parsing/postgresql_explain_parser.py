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
import subprocess
import os
from os.path import exists


@dataclass
class Node:
    type: str
    id: int
    params: list
    inputs: list
    tables: list


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
            all_nodes[current_node_counter] = Node(
                "Aggregate", current_node_counter, node_data['Output'], [], [])
        elif (node_data['Node Type'] == "Merge Join"):
            all_nodes[current_node_counter] = Node(
                "Join", current_node_counter, [
                    node_data['Merge Cond']], [], [])
            child_to_parent_counter = counter[0]
            counter[0] += 1
            all_nodes[child_to_parent_counter] = Node(
                "Filter", child_to_parent_counter, [
                    node_data['Join Filter']], [current_node_counter], [])
        elif (node_data['Node Type'] == "Seq Scan"):
            all_nodes[current_node_counter] = Node(
                "Filter", current_node_counter, [
                    node_data['Filter']], [-1], [
                    node_data['Alias']])
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
    single_chars = {"(", ")", "*"}
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
        elif (len(split_tokens) != 1):
            trimmed_tokens.append(split_tokens[1].upper())
        else:
            for char in removable_characters:
                token = token.replace(char, '')
            split_tokens = token.split(',')
            for new_token in split_tokens:
                trimmed_tokens.append(new_token)

    node.params = trimmed_tokens


def ParseJoinNode(all_nodes, key):
    removable_tokens = {'(', '=', ')'}
    new_tokens = []
    for token in all_nodes[key].params:
        if (token not in removable_tokens):
            new_tokens.append(token)
    # Assuming they are in the correct order
    all_nodes[key].params = new_tokens


def ParseFilterNode(all_nodes, key):
    reverse_notation = GetReversePolishNotation(all_nodes[key].params)
    all_nodes[key].params = reverse_notation


def GetReversePolishNotation(tokens):
    # Shunting Yard
    stack = []
    result = []
    operations = {
        "sum",
        "-",
        "+",
        "*",
        "/",
        "=",
        "!=",
        "<",
        ">",
        "<=",
        ">=",
        "AND",
        "OR",
        "NOT"
        "avg",
        "min",
        "max",
        "ANY"}
    any_stack = []
    is_any = False
    for token in tokens:
        if token in operations or token == '(':
            if token == "ANY":
                is_any = True
                # Assuming we have = at the top of the stack. And that the
                # column is before =
            else:
                # Assuming everything bracketed
                stack.append(token)
        elif token == ')':
            popped = stack.pop()
            while popped != '(':
                if is_any:
                    raise RuntimeError("Unknown error")
                result.append(popped)
                popped = stack.pop()
            if is_any:
                # Assuming we have = at the top of the stack.
                equals_token = stack.pop()
                equals_first_arg = result.pop()
                number_of_any_args = len(any_stack)
                while any_stack:
                    any_arg = any_stack.pop()
                    result.append(equals_first_arg)
                    result.append(any_arg)
                    result.append(equals_token)
                for i in range(number_of_any_args - 1):
                    stack.append("OR")
                is_any = False
        else:
            if is_any:
                any_stack.append(token)
            else:
                result.append(token)
    stack_size = len(stack)
    for i in range(stack_size):
        result.append(stack.pop())
    # print("TOKENS:")
    # print(tokens)
    # print("vs")
    # print(result)
    # print()
    return result


def ParseAggregateNode(all_nodes, key, counter):
    reverse_notation = GetReversePolishNotation(all_nodes[key].params)
    all_nodes[key].params = reverse_notation
    operations = {
        "sum",
        "-",
        "+",
        "*",
        "/",
        "avg",
        "min",
        "max"}
    operand_count = {
        "sum": 1,
        "-": 2,
        "+": 2,
        "*": 2,
        "/": 2,
        "avg": 1,
        "min": 1,
        "max": 1}
    stack = []
    new_operations = []
    for token in all_nodes[key].params:
        if token in operations:
            if operand_count[token] == 2:
                second_operand = stack.pop()
                first_operand = stack.pop()
            else:
                first_operand = stack.pop()
                second_operand = "ERROR"
            if token == "sum":
                new_operations.append(
                    {"name": "Aggregation", "params": [first_operand]})
                # print("sum:" + first_operand)
            elif token == "-":
                new_operations.append({"name": "Addition", "params": [
                    second_operand, "TRUE", first_operand]})
                # print(first_operand + "-" + second_operand)
                stack.append(second_operand)
            elif token == "*":
                new_operations.append({"name": "Multiplication", "params": [
                    first_operand, second_operand, "TEMP_MUL"]})
                # print(first_operand + "*" + second_operand)
                stack.append("TEMP_MUL")
            else:
                raise RuntimeError("Incorrect operation")
        else:
            stack.append(token)
    # Remove current
    current = all_nodes[key]
    current_inputs = current.inputs
    current_tables = current.tables
    del all_nodes[key]
    for operation in new_operations:
        # Not really needed but just in case
        if (operation["name"] == "Aggregation"):
            params = operation["params"]
        elif (operation["name"] == "Addition"):
            params = operation["params"]
        elif (operation["name"] == "Multiplication"):
            params = operation["params"]
        else:
            raise RuntimeError("Incorrect operation")
        all_nodes[counter[0]] = Node(operation["name"], counter[0],
                                     params, current_inputs, current_tables)
        current_inputs = [counter[0]]
        current_tables = []
        counter[0] += 1


def PrintFilterAPICalls(params):
    operations = {
        "=",
        "!=",
        "<",
        ">",
        "<=",
        ">=",
        "AND",
        "OR",
        "NOT"}
    stack = []
    counter = 0
    for token in params:
        if token in operations:
            if token == "NOT":
                first_operand = stack.pop()
                relation_name = "relation_" + str(counter)
                counter += 1
                print(f"{relation_name} : {token} {first_operand}")
                stack.append(relation_name)
            else:
                second_operand = stack.pop()
                first_operand = stack.pop()
                relation_name = "relation_" + str(counter)
                counter += 1
                print(f"{relation_name} : {first_operand} {token} {second_operand}")
                stack.append(relation_name)
        else:
            stack.append(token)
    print("STACK:")
    print(stack)


def AddFilterCondiditions(params, json_data, counter, filter_key):
    operations = {
        "=",
        "!=",
        "<",
        ">",
        "<=",
        ">=",
        "AND",
        "OR",
        "NOT"}
    stack = []
    for token in params:
        if token in operations:
            if token == "NOT":
                first_operand = stack.pop()
                AddJSONKey(json_data, counter[0], "COMP",
                           [filter_key, token, first_operand])
            else:
                second_operand = stack.pop()
                first_operand = stack.pop()
                AddJSONKey(json_data, counter[0], "COMP",
                           [filter_key, token, first_operand, second_operand])
            stack.append(counter[0])
            counter[0] += 1
        else:
            stack.append(token)


def CheckTableIsExported(table_name):
    # Check for CPP parsing
    csv_filename = table_name + ".csv"
    if not exists(csv_filename):
        raise RuntimeError(f"{csv_filename} doesn't exist!")
        # TODO: Fix this automatically!
    return csv_filename


def GetTableRowCount(table_name, database_name):
    column_count_command = f"echo 'SELECT count(*) FROM {table_name}' | psql {database_name}"
    count_output = subprocess.check_output(column_count_command, shell=True, text=True)
    count_lines = count_output.split("\n")
    return count_lines[2].strip()


def GetDataType(datatype_string):
    size_params = datatype_string.split("(")
    datatype_name = size_params[0]
    data_size = 1
    if len(size_params) != 1:
        data_size = size_params[1][:-1]
    datatype_dict = {
        "bigint": "integer",
        "integer": "integer",
        "numeric": "decimal",
        "character": "varchar",
        "date": "date",
        "character varying": "varchar"}
    return (datatype_dict[datatype_name], data_size)


def AddTableColumnsToJSON(json_data, counter, table_name, database_name):
    # exported_table_name = CheckTableIsExported(table_name)
    exported_table_name = table_name + ".csv"

    row_count = GetTableRowCount(table_name, database_name)
    table_key = counter[0]
    counter[0] += 1
    columns_key = counter[0]
    counter[0] += 1
    AddJSONKey(json_data, table_key, "Table",
               [row_count, columns_key, exported_table_name])

    # Assume this works problem free - the programs exists and no file problems
    # TODO: Remove the assumption and do error checking!
    command = f"echo '\d {table_name}' | psql {database_name}"
    output_filename = f"{table_name}.txt"
    os.system(f"{command} > {output_filename}")
    with open(output_filename) as f:
        lines = f.readlines()[3:]

    columns_keys = []
    for line in lines:
        values = line.split("|")[:2]
        if (len(values) > 1):
            data_type, data_size = GetDataType(values[1].strip())
            AddJSONKey(json_data, counter[0], "Column",
                       [values[0].strip().upper(), data_size, data_type])
            columns_keys.append(counter[0])
            counter[0] += 1
    AddJSONKey(json_data, columns_key, "Columns", columns_keys)


def AddJSONKey(json_data, key, type, params):
    json_data[key] = {"type": type, "params": params}


def AddJSONData(all_nodes, key, json_data, counter, database_name):
    # Currently Seq Scan Filters are hardcoded to have tables
    # TODO: Improve explain reading to get tables from elsewhere
    if key in json_data:
        raise RuntimeError("Inserting key multiple times!")
    for input_i in range(len(all_nodes[key].inputs)):
        if all_nodes[key].inputs[input_i] == -1:
            if input_i >= len(all_nodes[key].tables):
                raise RuntimeError("Incorrect number of tables given")
            all_nodes[key].inputs[input_i] = counter[0]
            # TODO: Check if the same table is used for any other query - Need to also add the pointer there
            AddTableColumnsToJSON(json_data, counter, all_nodes[key].tables[input_i], database_name)

    if all_nodes[key].type == "Aggregate":
        AddJSONKey(json_data, key, "Aggregate", [all_nodes[key].inputs[0], all_nodes[key].params[0]])
    elif all_nodes[key].type == "Join":
        AddJSONKey(json_data, key, "Join",
                   [all_nodes[key].inputs[0], all_nodes[key].params[0], all_nodes[key].inputs[1],
                    all_nodes[key].params[1]])
    elif all_nodes[key].type == "Filter":
        AddJSONKey(json_data, key, "Filter",
                   [all_nodes[key].inputs[0]])
        AddFilterCondiditions(all_nodes[key].params, json_data, counter, key)
    elif all_nodes[key].type == "Multiplication":
        AddJSONKey(json_data, key, "Multiplication",
                   [all_nodes[key].inputs[0], all_nodes[key].params[0], all_nodes[key].params[1],
                    all_nodes[key].params[2]])
    elif all_nodes[key].type == "Addition":
        AddJSONKey(json_data, key, "Addition",
                   [all_nodes[key].inputs[0], all_nodes[key].params[0], all_nodes[key].params[1],
                    all_nodes[key].params[2]])


def PrintAPICalls(all_nodes, key):
    if all_nodes[key].type == "Aggregate":
        print(f"RegisterAggregation({all_nodes[key].params[0]})")
    elif all_nodes[key].type == "Join":
        print(
            f"RegisterJoin({all_nodes[key].params[0]}, {all_nodes[key].params[1]})")
    elif all_nodes[key].type == "Filter":
        PrintFilterAPICalls(all_nodes[key].params)
    elif all_nodes[key].type == "Multiplication":
        print(
            f"RegisterMultiplication({all_nodes[key].params[0]}, {all_nodes[key].params[1]}, {all_nodes[key].params[2]})")
    elif all_nodes[key].type == "Addition":
        print(
            f"RegisterAddition({all_nodes[key].params[0]}, {all_nodes[key].params[1]}, {all_nodes[key].params[2]})")


def CheckPostgreSQL(database_name):
    # TODO: Also check that the database exists
    print("Checking PostgreSQL version:")
    result = subprocess.run(["psql", "--version"])
    try:
        result.check_returncode()
    except:
        print("No PostgreSQL!")


def GetExplainJSON(database_name, query_file):
    with open(query_file) as f:
        query = f.read()
    explain_command = f"EXPLAIN (VERBOSE TRUE, FORMAT JSON) {query}"
    command = f"echo \"{explain_command}\" | psql -t -A {database_name}"
    result = "temp"
    output_filename = f"{result}.json"
    os.system(f"{command} > {output_filename}")
    return output_filename


def main(argv):
    # argv is supposed to be the query in a file and the database name.
    # database_name = "tpch_001"
    database_name = argv[0]
    CheckPostgreSQL(database_name)

    # query_file = "q19.txt"
    query_file = argv[1]
    explain_output = GetExplainJSON(database_name, query_file)
    with open(explain_output) as graph_json:
        input_query_graph = json.load(graph_json)

    # for independent_query in input_query_graph:
    #     print_node(independent_query["Plan"])

    all_nodes = dict()
    counter = [0]
    for independent_query in input_query_graph:
        SetInitialNodes(independent_query["Plan"], all_nodes, counter, -1)

    # Let's assume we don't explicitly have to deal with table checking.

    # Then the params are a lot cleaner and then you can deal with the
    # notation:
    orig_nodes = all_nodes.copy()
    for key in orig_nodes.keys():
        # Remove all spaces
        # Remove all "\"
        # Make into a list of tokens!
        # If letter keep collecting until it's no longer a letter.
        # print(f"{key}:{all_nodes[key]}")
        TokenizeParams(all_nodes[key])
        # print(f"{key}:{all_nodes[key]}")
        if (all_nodes[key].type == "Aggregate"):
            ParseAggregateNode(all_nodes, key, counter)
        elif (all_nodes[key].type == "Join"):
            ParseJoinNode(all_nodes, key)
        elif (all_nodes[key].type == "Filter"):
            ParseFilterNode(all_nodes, key)
        else:
            raise RuntimeError("Incorrect type!")
        # print(f"{key}:{all_nodes[key]}")

    json_data = dict()
    for key in all_nodes.keys():
        pass
        # print(f"{key}:{all_nodes[key]}")
        # PrintAPICalls(all_nodes, key)
        AddJSONData(all_nodes, key, json_data, counter, database_name)

    # for key in json_data.keys():
    #     print(f"{key}:{json_data[key]}")
    result_file = 'parsed.json'
    with open(result_file, 'w') as fp:
        json.dump(json_data, fp)
    print(f"Data in {result_file}")


if __name__ == '__main__':
    main(sys.argv[1:])
