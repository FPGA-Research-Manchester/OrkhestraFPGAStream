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


def main(argv):
    argv.append("q19.json")
    with open(argv[0]) as graph_json:
        input_query_graph = json.load(graph_json)

    for independent_query in input_query_graph:
        print_node(independent_query["Plan"])


if __name__ == '__main__':
    main(sys.argv[1:])
