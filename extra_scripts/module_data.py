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

import sys
import json


# Possibly generate resource string
# Possibly generate the resource requirements

# Give resource string in
# Give a list of resource requirements for a dict of modules.
# Get all substrings that meet the requirements.
# Choose the smallest ones out of them for each requirement.
# Give each a unique name - module name + slack + index
def main(argv):
    # with open("test.json", 'r') as read_file:
    #     data = json.load(read_file)

    resource_string = "MMDMDBMMDBMMDMDBMMDBMMDMDBMMDBM"

    all_substrings = collect_all_substrings(resource_string)

    resource_requirements = {
        "module_1_easy": [{"M": 2, "D": 1, }],
        "module_2_easy": [{"M": 3, "D": 1, }],
        "module_3_easy": [{"M": 2, "D": 2, }],
        "module_4_hard": [{"M": 2, "D": 1, "B": 1}],
        "module_5_hard": [{"M": 3, "D": 3}],
        "module_6_hard": [{"M": 1, "D": 1, "B": 2}]
    }

    data = dict()
    # Find all substrings containing the required resources
    for module in resource_requirements.keys():
        data[module] = dict()
        data[module]["bitstreams"] = dict()
        max_slack = 0
        for requirement in resource_requirements[module]:
            module_substrings = get_module_substrings_with_requirements(all_substrings, requirement, max_slack)
            for substring_index in range(len(module_substrings)):
                bitstream_name = module + "_" + str(module_substrings[substring_index][0]) + "_" + str(substring_index)
                data[module]["bitstreams"][bitstream_name] = {"string": module_substrings[substring_index][1],
                                                              "is_backwards": False}

    add_position_and_length(data, resource_string)
    add_start_locations(data, len(resource_string))

    with open('test_result.json', 'w') as write_file:
        json.dump(data, write_file, indent=4, sort_keys=True)


def collect_all_substrings(input_str):
    substrings = []
    for i in range(len(input_str)):
        current_string = ""
        for j in range(i, len(input_str)):
            current_string += input_str[j]
            substrings.append(current_string)
    return sorted(substrings, key=lambda x: len(x))


def add_start_locations(data, pr_region_length):
    for module in data.keys():
        data[module]["start_locations"] = [[] for position in range(pr_region_length)]
        for bitstream in data[module]["bitstreams"].keys():
            for location in data[module]["bitstreams"][bitstream]["locations"]:
                data[module]["start_locations"][location].append(bitstream)


def is_satisfying_string(substring, requirement):
    for resource in requirement.keys():
        if substring.count(resource) < requirement[resource]:
            return False
    return True


def get_module_substrings_with_requirements(all_substrings, requirement, max_slack):
    unique_substrings = list(dict.fromkeys(all_substrings))
    matching_substrings = []
    smallest_string_len = -1
    for substring in unique_substrings:
        if is_satisfying_string(substring, requirement):
            if smallest_string_len == -1:
                smallest_string_len = len(substring)
            if len(substring) > smallest_string_len + max_slack:
                break
            matching_substrings.append((len(substring) - smallest_string_len, substring))
    return matching_substrings


def add_position_and_length(data, resource_string):
    for module in data.keys():
        for bitstream in data[module]["bitstreams"].keys():
            module_string = data[module]["bitstreams"][bitstream]["string"]
            data[module]["bitstreams"][bitstream]["length"] = len(module_string)
            data[module]["bitstreams"][bitstream]["locations"] = [i for i in range(len(resource_string)) if
                                                                  resource_string.startswith(module_string, i)]


if __name__ == '__main__':
    main(sys.argv[1:])
