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
import statistics

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


def get_bitstream_operators(hw_library):
    all_bitstream_operators_map = dict()
    for operator in hw_library.keys():
        for bitstream in hw_library[operator]["bitstreams"].keys():
            all_bitstream_operators_map[bitstream] = operator
    return all_bitstream_operators_map


# How do I do a heatmap?
# Global & Per module: Count how many times the resource is covered.
# You can normalize between max and min hits to percentages.
#
def main(argv):
    global_start_locations = get_global_start_location(hw_library, len(resource_string))
    all_bitstream_operators_map = get_bitstream_operators(hw_library)
    global_resource_hit_counts = get_resource_hit_counts(global_start_locations, hw_library,
                                                         all_bitstream_operators_map, len(resource_string))
    global_resource_hit_counts = normalize_counts(global_resource_hit_counts)
    print("Global heatmap:")
    print_heatmap(global_resource_hit_counts)
    # Do the same for all other
    for module in hw_library.keys():
        global_resource_hit_counts = get_resource_hit_counts(hw_library[module]["start_locations"], hw_library,
                                                             all_bitstream_operators_map, len(resource_string))
        global_resource_hit_counts = normalize_counts(global_resource_hit_counts)
        print(module + " heatmap:")
        print_heatmap(global_resource_hit_counts)


def get_resource_hit_counts(global_start_locations, hw_library, all_bitstream_operators_map, region_size):
    hit_counts = [0 for i in range(region_size)]
    for location in range(len(global_start_locations)):
        for bitstream in global_start_locations[location]:
            for hit_location in range(location, location +
                                                hw_library[all_bitstream_operators_map[bitstream]]["bitstreams"][
                                                    bitstream]["length"]):
                hit_counts[hit_location] += 1
    return hit_counts


def normalize_counts(global_resource_hit_counts):
    return [float(i) / sum(global_resource_hit_counts) for i in global_resource_hit_counts]


def print_heatmap(global_resource_hit_counts):
    print(["{0:0.2f}".format(i) for i in global_resource_hit_counts])
    print(statistics.stdev(global_resource_hit_counts))
    print(sum(global_resource_hit_counts) / len(global_resource_hit_counts))
    # For modules that have a high std dev (More than 1/3 MEAN) it is better to try and add more modules.
    # Then Find rare and can common combinations and make some HW library out of them
    # Then check the heatmap of them
    # Then if heatmap is bad increase it to see if heatmap improves
    # Then the last thing to check is with the density - Does more evenly distributed density equal to better density?


if __name__ == '__main__':
    main(sys.argv[1:])
