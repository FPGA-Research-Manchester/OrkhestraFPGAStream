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

import sys

resource_string = "MMDMDBMMDBMMDMDBMMDBMMDMDBMMDBM"
hw_library = {
    "Filter": {
        "bitstreams": {
            "filter81.bit": {"locations": (0, 10, 20), "length": 4, "capacity": (8, 1), "string": "MMDM",
                             "is_backwards": False},
            "filter84.bit": {"locations": (2, 12, 22), "length": 7, "capacity": (8, 4), "string": "DMDBMMD",
                             "is_backwards": False},
            "filter162.bit": {"locations": (0, 10, 20), "length": 5, "capacity": (16, 2), "string": "MMDMD",
                              "is_backwards": False},
            "filter164.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (16, 4), "string": "MMDMDBM",
                              "is_backwards": False},
            "filter321.bit": {"locations": (0, 10, 20), "length": 5, "capacity": (32, 1), "string": "MMDMD",
                              "is_backwards": False},
            "filter322.bit": {"locations": (2, 12, 22), "length": 7, "capacity": (32, 2), "string": "DMDBMMD",
                              "is_backwards": False},
            "filter324.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (32, 4), "string": "MMDMDBMMDB",
                              "is_backwards": False}},
        "start_locations": [['filter81.bit', 'filter162.bit', 'filter164.bit', 'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [],
                            ['filter81.bit', 'filter162.bit', 'filter164.bit',
                             'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [],
                            ['filter81.bit', 'filter162.bit', 'filter164.bit',
                             'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [], []],
    }, "Linear Sort": {
        "bitstreams": {
            "linear512.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (512,), "string": "MMDMDBMMDB",
                              "is_backwards": False},
            "linear1024.bit": {"locations": (6, 16), "length": 14, "capacity": (1024,), "string": "MMDBMMDMDBMMDB",
                               "is_backwards": False}},
        "start_locations": [['linear512.bit'], [], [], [], [], [], ['linear1024.bit'], [], [], [],
                            ['linear512.bit'], [],
                            [], [], [], [], ['linear1024.bit'], [], [], [], [
                                'linear512.bit'], [], [], [], [], [],
                            [],
                            [], [], [], []],
    }, "Merge Sort": {
        "bitstreams": {
            "mergesort32.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (32,), "string": "MMDMDBM",
                                "is_backwards": False},
            "mergesort64.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (64,), "string": "MMDMDBMMDB",
                                "is_backwards": False},
            "mergesort128.bit": {"locations": (8, 18), "length": 12, "capacity": (128,), "string": "DBMMDMDBMMDB",
                                 "is_backwards": False}},
        "start_locations": [['mergesort32.bit', 'mergesort64.bit'], [], [], [], [], [], [], [],
                            ['mergesort128.bit'], [],
                            ['mergesort32.bit', 'mergesort64.bit'], [
                            ], [], [], [], [], [], [],
                            ['mergesort128.bit'], [],
                            ['mergesort32.bit', 'mergesort64.bit'], [], [], [], [], [], [], [], [], [], []],
    }, "Merge Join": {
        "bitstreams": {
            "join.bit": {"locations": (0, 10, 20), "length": 6, "capacity": (), "string": "MMDMDB",
                         "is_backwards": False}},
        "start_locations": [['join.bit'], [], [], [], [], [], [], [], [], [], ['join.bit'], [], [], [], [], [], [],
                            [],
                            [], [], ['join.bit'], [], [], [], [], [], [], [], [], [], []],
    }, "Addition": {
        "bitstreams": {
            "addition.bit": {"locations": (0, 10, 20), "length": 4, "capacity": (), "string": "MMDM",
                             "is_backwards": False}},
        "start_locations": [['addition.bit'], [], [], [], [], [], [], [], [], [], ['addition.bit'], [], [], [], [],
                            [], [], [], [],
                            [], ['addition.bit'], [], [], [], [], [], [], [], [], [], []],
    }, "Multiplier": {
        "bitstreams": {
            "multiplier.bit": {"locations": (4, 14, 24), "length": 7, "capacity": (), "string": "DBMMDBM",
                               "is_backwards": False}},
        "start_locations": [[], [], [], [], ['multiplier.bit'], [], [], [], [], [], [], [], [], [],
                            ['multiplier.bit'], [], [],
                            [], [], [], [], [], [], [], ['multiplier.bit'], [], [], [], [], [], []],
    }, "Global Sum": {
        "bitstreams": {
            "globalsum.bit": {"locations": (2, 12, 22), "length": 3, "capacity": (), "string": "DMD",
                              "is_backwards": False}},
        "start_locations": [[], [], ['globalsum.bit'], [], [], [], [], [], [], [], [], [], ['globalsum.bit'], [],
                            [], [], [], [], [],
                            [], [], [], ['globalsum.bit'], [], [], [], [], [], [], [], []],
    }
}


def get_global_start_location(hw_library, pr_region_length):
    global_start_locations = [[] for position in range(pr_region_length)]
    # Assuming all bitstream names are different.
    for operator in hw_library.values():
        for location in range(pr_region_length):
            for bitstream in operator["start_locations"][location]:
                global_start_locations[location].append(bitstream)
    return global_start_locations


def find_fitting_configuration(all_configurations, current_configuration, all_start_locations, current_location,
                               max_location, hw_library, all_bitstream_operators_map):
    if current_location < max_location:
        for bitstream in all_start_locations[current_location]:
            new_configuration = current_configuration.copy()
            new_configuration.append((bitstream, current_location))
            find_fitting_configuration(all_configurations, new_configuration, all_start_locations,
                                       current_location +
                                       hw_library[all_bitstream_operators_map[bitstream]]["bitstreams"][bitstream][
                                           "length"],
                                       max_location, hw_library, all_bitstream_operators_map)

        find_fitting_configuration(all_configurations, current_configuration, all_start_locations, current_location + 1,
                                   max_location, hw_library, all_bitstream_operators_map)
    else:
        all_configurations.append(current_configuration)


def get_bitstream_operators(hw_library):
    all_bitstream_operators_map = dict()
    for operator in hw_library.keys():
        for bitstream in hw_library[operator]["bitstreams"].keys():
            all_bitstream_operators_map[bitstream] = operator
    return all_bitstream_operators_map


def find_all_configurations(hw_library, resource_string):
    pr_region_length = len(resource_string)
    global_start_locations = get_global_start_location(hw_library, pr_region_length)
    all_configurations = []  # List of pairs (bitstream name and location)
    all_bitstream_operators_map = get_bitstream_operators(hw_library)
    find_fitting_configuration(all_configurations, [], global_start_locations, 0, pr_region_length, hw_library,
                               all_bitstream_operators_map)
    return all_configurations


def sort_based_on_density(list_of_configurations):
    return sorted(list_of_configurations, key=lambda x: len(x), reverse=True)


def print_top(list_of_configurations):
    count = 0
    limit = 10
    while count < limit and count < len(list_of_configurations):
        print(list_of_configurations[count])
        count += 1


def remove_bitstream_repetitions(list_of_configurations):
    filtered_configurations = list_of_configurations.copy()
    for configuration in list_of_configurations:
        seen_bitstreams = set()
        for bitstream, position in configuration:
            if bitstream in seen_bitstreams:
                filtered_configurations.remove(configuration)
                break
            else:
                seen_bitstreams.add(bitstream)
    return filtered_configurations


def remove_operator_repetitions(list_of_configurations, hw_library):
    filtered_configurations = list_of_configurations.copy()
    all_bitstream_operators_map = get_bitstream_operators(hw_library)
    for configuration in list_of_configurations:
        seen_operators = set()
        for bitstream, position in configuration:
            if all_bitstream_operators_map[bitstream] in seen_operators:
                filtered_configurations.remove(configuration)
                break
            else:
                seen_operators.add(all_bitstream_operators_map[bitstream])
    return filtered_configurations


# Given a hardware library and a resource string.
# Print the 10 most dense ways to pack them.
# With repeat and without repeat
def main(argv):
    list_of_all_configurations_with_repetitions = find_all_configurations(hw_library, resource_string)
    list_of_all_configurations_with_repetitions = sort_based_on_density(list_of_all_configurations_with_repetitions)
    print("All:")
    print_top(list_of_all_configurations_with_repetitions)
    list_of_configurations_without_repetitions = remove_bitstream_repetitions(
        list_of_all_configurations_with_repetitions)
    print("No bitstream repeats:")
    print_top(list_of_configurations_without_repetitions)
    list_of_configurations_without_repetitions = remove_operator_repetitions(list_of_configurations_without_repetitions,
                                                                             hw_library)
    print("No operator repeats:")
    print_top(list_of_configurations_without_repetitions)


if __name__ == '__main__':
    main(sys.argv[1:])
