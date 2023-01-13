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
import json
import sys
from dataclasses import dataclass
import subprocess
import os
from os.path import exists


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

def GetExplainJSON(filename, executable, catalog_name, schema_name):
    with open(filename) as f:
        query = f.read()
    explain_command = f"EXPLAIN {query};"
    command = f"java -jar {executable} --server localhost:8090 --catalog {catalog_name} --schema {schema_name} --execute \"{explain_command}\""
    result = "EXPLAIN"
    output_filename = f"{result}.txt"
    print(f"DBMS command: \n {command}")
    os.system(f"{command} > {output_filename}")
    print(f"EXPLAIN output in: {output_filename}")
    return output_filename


def create_initial_nodes_dict(filename, executable, catalog_name, schema_name):
    explain_output = GetExplainJSON(filename, executable, catalog_name, schema_name)

    with open(explain_output, 'r') as f:
        lines = [line.rstrip().replace('"', '')
                 for line in f if line.rstrip().replace('"', '').lstrip()]

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
            key, value = trimmed_line.replace('│', '').split(":=")
            all_initial_nodes[last_node_id].params[key.strip()] = value.strip()
            continue
        if not all_initial_nodes:
            all_initial_nodes[last_node_id] = InitialNode(
                trimmed_line, last_node_id, dict(), [], [])
            continue
        # Need to make a new node
        if '└─' not in trimmed_line:
            raise RuntimeError(f"Unexpected line:{trimmed_line}")
        current_indentation = len(line) - len(trimmed_line)
        trimmed_line = trimmed_line.replace('└─', '').lstrip()
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


def TokenizeParams(node):
    current_token = ""
    all_tokens = []
    quotes_active = False
    skipped_chars = {"\\", "[", "]", "'", "{", "}", "\"", ","}
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

    value_tokens = []
    skipped_token_count = 0
    # First we process tokens with ::
    for token_i in range(len(all_tokens)):
        if skipped_token_count != 0:
            skipped_token_count -= 1
        else:
            token = all_tokens[token_i]
            # TODO: Taking some assumptions here - Make this more robust!
            if token[0] == ":":
                if token == "::timestamp" and all_tokens[token_i +
                                                         1] == "without":
                    skipped_token_count = 3
                    value_tokens[-1] = value_tokens[-1].split(' ')[0]
            else:
                value_tokens.append(token)

    trimmed_tokens = []
    removable_characters = {"\"", "{", "}"}
    for token in value_tokens:
        split_tokens = token.split('.')
        if (len(split_tokens) != 1):
            # Is a numeric token not a table + column token.
            if (split_tokens[0].isnumeric()):
                trimmed_tokens.append(split_tokens[0] + "." + split_tokens[1])
            else:
                trimmed_tokens.append(split_tokens[1])
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


def ParseSortNode(all_nodes, key):
    pass


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
        "AVG",
        "MIN",
        "MAX",
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
        "AVG",
        "MIN",
        "MAX"}
    operand_count = {
        "sum": 1,
        "-": 2,
        "+": 2,
        "*": 2,
        "/": 2,
        "AVG": 1,
        "MIN": 1,
        "MAX": 1}
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
                    {"name": "Aggregate", "params": [first_operand]})
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
        if (operation["name"] == "Aggregate"):
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


def CheckTableIsExported(table_name):
    # Check for CPP parsing
    csv_filename = table_name + ".csv"
    # if not exists(csv_filename):
    #     raise RuntimeError(f"{csv_filename} doesn't exist!")
    # TODO: Fix this automatically!
    return csv_filename


def GetTableRowCount(table_name, executable, catalog_name, schema_name):
    query = f"SELECT count(*) FROM {table_name};"
    command = f"java -jar {executable} --server localhost:8090 --catalog {catalog_name} --schema {schema_name} --execute \"{query}\""
    count_output = subprocess.check_output(
        command, shell=True, text=True)
    count_lines = count_output.split("\n")
    value = int(count_lines[0].replace('"',''))
    return value


def GetDataType(datatype_string):
    size_params = datatype_string.split("(")
    datatype_name = size_params[0]
    data_size = 1
    if len(size_params) != 1:
        data_size = size_params[1][:-1]
    datatype_dict = {
        "bigint": "integer",
        "integer": "integer",
        "double": "decimal",
        "varchar": "varchar",
        "date": "date",}
    if datatype_name not in datatype_dict:
        raise RuntimeError(f"Unsopperted type: {datatype_name}")
    return (datatype_dict[datatype_name], data_size)

def AddTableColumnsToJSON(json_data, counter, table_name, executable, catalog_name, schema_name):
    exported_table_name = CheckTableIsExported(table_name)
    # exported_table_name = table_name + ".csv"

    row_count = GetTableRowCount(table_name, executable, catalog_name, schema_name)
    table_key = counter[0]
    counter[0] += 1
    columns_key = counter[0]
    counter[0] += 1
    AddJSONKey(json_data, table_key, "Table",
               [row_count, columns_key, exported_table_name])

    # Assume this works problem free - the programs exists and no file problems
    # TODO: Remove the assumption and do error checking!
    query = f"describe {table_name};"
    command = f"java -jar {executable} --server localhost:8090 --catalog {catalog_name} --schema {schema_name} --execute \"{query}\""
    output_filename = f"{table_name}.txt"
    os.system(f"{command} > {output_filename}")
    with open(output_filename) as f:
        lines = f.readlines()

    columns_keys = []
    for line in lines:
        values = [value.replace('"','') for value in line.split(",")[:2]]
        if (len(values) > 1):
            data_type, data_size = GetDataType(values[1])
            AddJSONKey(json_data, counter[0], "Column",
                       [values[0], data_size, data_type])
            columns_keys.append(counter[0])
            counter[0] += 1
        else:
            raise RuntimeError("Unexpected column!")
    AddJSONKey(json_data, columns_key, "Columns", columns_keys)


def AddJSONKey(json_data, key, type, params):
    json_data[key] = {"type": type, "params": params}


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

def AddJSONData(all_nodes, key, json_data, counter, executable, catalog_name, schema_name):
    # Currently Seq Scan Filters are hardcoded to have tables
    # TODO: Improve explain reading to get tables from elsewhere
    if key in json_data:
        raise RuntimeError("Inserting key multiple times!")
    for input_i in range(len(all_nodes[key].inputs)):
        if all_nodes[key].inputs[input_i] == -1:
            if input_i >= len(all_nodes[key].tables):
                raise RuntimeError("Incorrect number of tables given")
            all_nodes[key].inputs[input_i] = counter[0]
            # TODO: Check if the same table is used for any other query - Need
            # to also add the pointer there
            AddTableColumnsToJSON(
                json_data,
                counter,
                all_nodes[key].tables[input_i].split(":")[1],
                executable, catalog_name, schema_name)

    if all_nodes[key].type == "Aggregate":
        AddJSONKey(
            json_data, key, "Aggregate", [
                all_nodes[key].inputs[0], all_nodes[key].params[0]])
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
    elif all_nodes[key].type == "Sort":
        AddJSONKey(json_data, key, "Sort",
                   [all_nodes[key].inputs[0], all_nodes[key].params[0]])
    else:
        raise RuntimeError(f"Unsopperted type: {all_nodes[key].type}")

def parsing_nodes(nodes, executable, catalog_name, schema_name):
    keys = nodes.keys()
    counter = [list(keys)[-1]+1]
    orig_nodes = nodes.copy()
    for key in orig_nodes.keys():
        # Remove all spaces
        # Remove all "\"
        # Make into a list of tokens!
        # If letter keep collecting until it's no longer a letter.
        #print("BEFORE before")
        #print(f"{key}:{nodes[key]}")
        #print()
        TokenizeParams(nodes[key])
        # print("BEFORE:")
        # print(f"{key}:{all_nodes[key]}")
        # print()
        if (nodes[key].type == "Aggregate"):
            ParseAggregateNode(nodes, key, counter)
        elif (nodes[key].type == "Join"):
            ParseJoinNode(nodes, key)
        elif (nodes[key].type == "Filter"):
            ParseFilterNode(nodes, key)
        elif (nodes[key].type == "Sort"):
            ParseSortNode(nodes, key)
        else:
            raise RuntimeError(f"Incorrect type:{nodes[key]}")
        # print("AFTER:")
        # if (key in all_nodes):
        #     print(f"{key}:{all_nodes[key]}")
        #     print()
        # else:
        #     print("Removed")
        #     print()

    # raise RuntimeError("STOP!")
    json_data = dict()
    for key in nodes.keys():
        # pass
        #print(f"{key}:{nodes[key]}")
        # PrintAPICalls(nodes, key)
        AddJSONData(nodes, key, json_data, counter, executable, catalog_name, schema_name)
    # quit()

    return json_data


def CheckTrino(executable, catalog_name, database_name):
    # Possible command:
    # java  -jar /opt/openlookeng/resource/hetu-cli-*-executable.jar   --server localhost:8090 --catalog tpch --schema tiny --execute "select count(*) from part;"
    print("Checking DBMS executable version:")
    result = subprocess.run(["java", "-jar", executable, "--version"],
                            capture_output=True, encoding='UTF-8')
    try:
        result.check_returncode()
        print(result.stdout)
    except BaseException:
        raise RuntimeError("No DBMS!")
    output = subprocess.run(["java", "-jar", executable, "--server", "localhost:8090", "--execute", "show catalogs;"], capture_output=True, encoding='UTF-8')
    if catalog_name not in output.stdout:
        raise RuntimeError("No catalog: " + catalog_name)
    output = subprocess.run(["java", "-jar", executable, "--server", "localhost:8090", "--catalog",catalog_name,"--execute", "show schemas;"],
                            capture_output=True, encoding='UTF-8')
    if database_name not in output.stdout:
        raise RuntimeError("No schema: " + database_name)

def ExecuteSQL(argv):
    executable = argv[0]
    catalog_name = argv[1]
    database_name = argv[2]
    CheckTrino(executable, catalog_name, database_name)

    query_file = argv[3]
    with open(query_file) as f:
        query = f.read()
    command = f"java -jar {executable} --server localhost:8090 --catalog {catalog_name} --schema {database_name} --execute \"{query}\""
    result_file = argv[4]
    print(f"DBMS command: \n {command}")
    os.system(f"{command} > {result_file}")
    print(f"Results in: {result_file}")


def main(argv):
    # python3 trino_translate.py /opt/openlookeng/resource/hetu-cli-*-executable.jar tpch tiny q6_trino.txt result.json execute
    if len(argv) != 5:
        if len(argv) == 6 and argv[5] == "execute":
            ExecuteSQL(argv)
            return
        else:
            raise RuntimeError("Incorrect number of arguments!")

    # Executable is required to make it work with Presto, Trino, and openLooKeng
    executable = argv[0]
    catalog_name = argv[1]
    schema_name = argv[2]
    explain_input = argv[3]
    result_file = argv[4]
    # database_name = "tpch_001"
    # explain_input = "openLooKeng_input.txt"
    # result_file = 'parsed.json'
    # remove_lines_and_print(explain_input)
    initial_nodes = create_initial_nodes_dict(explain_input, executable, catalog_name, schema_name)
    initial_nodes = condense_initial_nodes_dict(initial_nodes)
    final_nodes = transform_to_final_nodes_dict(initial_nodes)

    # for key in final_nodes.keys():
    #     print(f"{key}:{final_nodes[key]}")
    #     print()

    json_data = parsing_nodes(final_nodes, executable, catalog_name, schema_name)

    # for key in json_data.keys():
    #     print(f"{key}:{json_data[key]}")

    with open(result_file, 'w') as fp:
        json.dump(json_data, fp)
    print(f"API function parameters in: {result_file}")




if __name__ == '__main__':
    main(sys.argv[1:])
