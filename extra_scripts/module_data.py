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
import itertools
import statistics
import copy

import seaborn as sns
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd


# Possibly generate resource string
# Possibly generate the resource requirements

# Give resource string in
# Give a list of resource requirements for a dict of modules.
# Get all substrings that meet the requirements.
# Choose the smallest ones out of them for each requirement.
# Give each a unique name - module name + slack + index


def make_resource_requirements(hw_library):
    requirements = dict()
    for operation in hw_library.keys():
        requirements[operation] = []
        for module in hw_library[operation]["bitstreams"].keys():
            module_string = hw_library[operation]["bitstreams"][module]["string"]
            requirements[operation].append({c: module_string.count(c) for c in set(module_string)})
    return requirements


def get_existing(bitstream_dict):
    existing = set()
    for bitstream in bitstream_dict.keys():
        existing.add(bitstream_dict[bitstream]["string"])
    return existing


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


def get_resource_hit_counts(
        global_start_locations, hw_library, all_bitstream_operators_map, region_size):
    hit_counts = [0 for i in range(region_size)]
    for location in range(len(global_start_locations)):
        for bitstream in global_start_locations[location]:
            for hit_location in range(location, location +
                                                hw_library[all_bitstream_operators_map[bitstream]]["bitstreams"][
                                                    bitstream]["length"]):
                hit_counts[hit_location] += 1
    return hit_counts


def normalize_counts(global_resource_hit_counts):
    return [float(i) / sum(global_resource_hit_counts)
            for i in global_resource_hit_counts]


def get_normalized_resource_popularity(target_hw_library, resource_string):
    global_start_locations = get_global_start_location(
        target_hw_library, len(resource_string))
    all_bitstream_operators_map = get_bitstream_operators(target_hw_library)
    global_resource_hit_counts = get_resource_hit_counts(global_start_locations, target_hw_library,
                                                         all_bitstream_operators_map, len(resource_string))
    global_resource_hit_counts = normalize_counts(global_resource_hit_counts)
    return global_resource_hit_counts


def add_additional_modules(all_substrings, current_hw_library, max_slack, resource_requirements, module_selection_func,
                           resource_string):
    # Find all substrings containing the required resources
    for module in resource_requirements.keys():
        for requirement_index in range(len(resource_requirements[module])):
            existing = get_existing(current_hw_library[module]["bitstreams"])
            module_substrings = get_module_substrings_with_requirements(
                all_substrings, resource_requirements[module][requirement_index], existing, max_slack)
            chosen_substring = module_selection_func(module_substrings, all_substrings, resource_string,
                                                     current_hw_library, module, requirement_index)
            add_bitstream_to_hw_library(chosen_substring, current_hw_library, module, requirement_index)
    # print("Updated:")
    # print(json.dumps(data, indent=4))


def add_bitstream_to_hw_library(chosen_substring, current_hw_library, module, requirement_index):
    bitstream_name = module + "_" + str(requirement_index) + "_" + \
                     str(chosen_substring[0])
    current_hw_library[module]["bitstreams"][bitstream_name] = {"string": chosen_substring[1],
                                                                "is_backwards": False}


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
        data[module]["start_locations"] = [[]
                                           for position in range(pr_region_length)]
        for bitstream in data[module]["bitstreams"].keys():
            for location in data[module]["bitstreams"][bitstream]["locations"]:
                data[module]["start_locations"][location].append(bitstream)


def is_satisfying_string(substring, requirement):
    for resource in requirement.keys():
        if substring.count(resource) < requirement[resource]:
            return False
    return True


def get_module_substrings_with_requirements(
        all_substrings, requirement, existing, max_slack):
    unique_substrings = list(dict.fromkeys(all_substrings))
    matching_substrings = []
    smallest_string_len = -1
    minimum_size = 0
    while not matching_substrings:
        for substring in unique_substrings:
            if len(substring) > minimum_size:
                if is_satisfying_string(substring, requirement) and substring not in existing:
                    if smallest_string_len == -1:
                        smallest_string_len = len(substring)
                    if len(substring) > smallest_string_len + max_slack:
                        break
                    matching_substrings.append(
                        (len(existing), substring))
        minimum_size += 1

    return matching_substrings


def add_position_and_length(data, resource_string):
    for module in data.keys():
        for bitstream in data[module]["bitstreams"].keys():
            module_string = data[module]["bitstreams"][bitstream]["string"]
            data[module]["bitstreams"][bitstream]["length"] = len(
                module_string)
            data[module]["bitstreams"][bitstream]["locations"] = [i for i in range(len(resource_string)) if
                                                                  resource_string.startswith(module_string, i)]


def choose_based_on_histogram(module_substrings, all_substrings, resource_string, hw_library, module,
                              requirement_index):
    string_counts = {pattern: all_substrings.count(pattern) for pattern in set(all_substrings)}

    max_placements = 0
    chosen_substring = module_substrings[0]
    for substring in module_substrings:
        if string_counts[substring[1]] > max_placements:
            chosen_substring = substring
            max_placements = string_counts[substring[1]]

    return chosen_substring


# We need to get the popularity.
def choose_based_on_heatmap(module_substrings, all_substrings, resource_string, hw_library, module, requirement_index):
    std_devs = dict()
    for substring in module_substrings:
        current_hw_library = copy.deepcopy(hw_library)
        add_bitstream_to_hw_library(substring, current_hw_library, module, requirement_index)
        add_position_and_length(current_hw_library, resource_string)
        add_start_locations(current_hw_library, len(resource_string))
        global_resource_hit_counts = get_normalized_resource_popularity(current_hw_library, resource_string)
        std_devs[substring[1]] = statistics.stdev(global_resource_hit_counts)

    # print("Orig:")
    # print(json.dumps(std_devs, indent=4))
    # print(min(std_devs, key=std_devs.get))

    min_pattern = min(std_devs, key=std_devs.get)
    for substring in module_substrings:
        if substring[1] == min_pattern:
            return substring

    raise ValueError("Error")


def main(argv):
    # with open("test.json", 'r') as read_file:
    #     data = json.load(read_file)

    resource_string = "MMDMDBMMDBMMDMDBMMDBMMDMDBMMDBM"

    all_substrings = collect_all_substrings(resource_string)

    initial_modules = {
        "filter": ["MMDM", "DMDBMMD", "MMDMD", "MMDMDBM", "MMDMD", "DMDBMMD", "MMDMDBMMDB"],
        "lsort": ["MMDMDBMMDB", "MMDBMMDMDBMMDB"],
        "msort": ["MMDMDBM", "MMDMDBMMDB", "DBMMDMDBMMDB"],
        "mjoin": ["MMDMDB", "MMDMDBMMDB", "DBMMDMDBMMDB"]
    }

    hw_library = dict()
    for module in initial_modules.keys():
        hw_library[module] = dict()
        hw_library[module]["bitstreams"] = dict()
        for substring_index in range(len(initial_modules[module])):
            bitstream_name = module + "_" + str(substring_index)
            hw_library[module]["bitstreams"][bitstream_name] = {"string": initial_modules[module][substring_index],
                                                                "is_backwards": False}

    # print("Orig:")
    # print(json.dumps(hw_library, indent=4))

    resource_requirements = make_resource_requirements(hw_library)

    # print("Reqs:")
    # print(json.dumps(resource_requirements, indent=4))

    # resource_requirements = {
    #     "module_1_hard": [{"M": 3, "D": 1, "B": 1}],
    #     "module_2_hard": [{"M": 7, "D": 3, "B": 3}],
    #     "module_3_hard": [{"B": 3}],
    #     "module_4_hard": [{"D": 4}],
    #     "module_5_hard": [{"M": 7}]
    # }

    # hardcoded to 0 for now
    max_slack = 0
    additional_module_count = 10
    global_std_devs = []
    global_frames = []
    frames = []
    std_devs = []
    current_hw_library = copy.deepcopy(hw_library)
    find_std_devs_after_adding_modules(additional_module_count, all_substrings, current_hw_library, max_slack,
                                       resource_requirements, resource_string, std_devs, choose_based_on_histogram, frames)
    print(frames)
    global_std_devs.append(std_devs)
    global_frames.append(frames)

    std_devs = []
    frames = []
    current_hw_library = copy.deepcopy(hw_library)
    find_std_devs_after_adding_modules(additional_module_count, all_substrings, current_hw_library, max_slack,
                                       resource_requirements, resource_string, std_devs, choose_based_on_heatmap, frames)
    print(frames)
    global_std_devs.append(std_devs)
    global_frames.append(frames)

    max_slack = 1
    std_devs = []
    frames = []
    current_hw_library = copy.deepcopy(hw_library)
    find_std_devs_after_adding_modules(additional_module_count, all_substrings, current_hw_library, max_slack,
                                       resource_requirements, resource_string, std_devs, choose_based_on_histogram, frames)
    print(frames)
    global_std_devs.append(std_devs)
    global_frames.append(frames)

    std_devs = []
    frames = []
    current_hw_library = copy.deepcopy(hw_library)
    find_std_devs_after_adding_modules(additional_module_count, all_substrings, current_hw_library, max_slack,
                                       resource_requirements, resource_string, std_devs, choose_based_on_heatmap, frames)
    print(frames)
    global_std_devs.append(std_devs)
    global_frames.append(frames)

    max_slack = 2
    std_devs = []
    frames = []
    current_hw_library = copy.deepcopy(hw_library)
    find_std_devs_after_adding_modules(additional_module_count, all_substrings, current_hw_library, max_slack,
                                       resource_requirements, resource_string, std_devs, choose_based_on_histogram, frames)
    print(frames)
    global_std_devs.append(std_devs)
    global_frames.append(frames)

    std_devs = []
    frames = []
    current_hw_library = copy.deepcopy(hw_library)
    find_std_devs_after_adding_modules(additional_module_count, all_substrings, current_hw_library, max_slack,
                                       resource_requirements, resource_string, std_devs, choose_based_on_heatmap, frames)
    print(frames)
    global_std_devs.append(std_devs)
    global_frames.append(frames)

    std_dev_dict = []
    frames_dict = []
    for run_i in range(len(global_std_devs)):
        for std_dev_i in range(len(global_std_devs[run_i])):
            cur_type = "histogram" if run_i % 2 == 0 else "heatmap"
            module_additions = std_dev_i
            additional_slack = run_i // 2
            std_dev_dict.append(
                {"Std dev": global_std_devs[run_i][std_dev_i], "Additional slack": additional_slack, "Type": cur_type,
                 "Module additions": module_additions})
            frames_dict.append(
                {"MB": global_frames[run_i][std_dev_i]/1000000, "Additional slack": additional_slack, "Type": cur_type,
                 "Module additions": module_additions})

    std_dev_df = pd.DataFrame(std_dev_dict)
    frames_df = pd.DataFrame(frames_dict)

    #sns.set_theme()
    sns.set_style("whitegrid")
    #sns.set(font_scale=2)
    g=sns.lineplot(data=std_dev_df, hue="Type", x="Module additions", y="Std dev", style="Additional slack")
    g.set_xticks(range(11))
    g.set_title("Standard deviation of module library extensions")
    plt.show()

    sns.set_style("whitegrid")
    # sns.set(font_scale=2)
    g = sns.lineplot(data=frames_df, hue="Type", x="Module additions", y="MB", style="Additional slack")
    g.set_xticks(range(11))
    g.set_title("Module library size after extensions")
    plt.show()

    # with open('test_result.json', 'w') as write_file:
    #     json.dump(data, write_file, indent=4, sort_keys=True)

    # To print out potential task orders
    # print(list(itertools.permutations(list(resource_requirements.keys()))))


def find_std_devs_after_adding_modules(additional_module_count, all_substrings, hw_library, max_slack,
                                       resource_requirements, resource_string, std_devs, module_selection_func, frames):
    add_position_and_length(hw_library, resource_string)
    add_start_locations(hw_library, len(resource_string))
    global_resource_hit_counts = get_normalized_resource_popularity(hw_library, resource_string)
    std_devs.append(statistics.stdev(global_resource_hit_counts))
    frame_size = 372
    frames.append(frame_size * get_frame_count(hw_library))
    for i in range(additional_module_count):
        add_additional_modules(all_substrings, hw_library, max_slack, resource_requirements, module_selection_func,
                               resource_string)

        add_position_and_length(hw_library, resource_string)
        add_start_locations(hw_library, len(resource_string))

        global_resource_hit_counts = get_normalized_resource_popularity(hw_library, resource_string)
        std_devs.append(statistics.stdev(global_resource_hit_counts))
        frames.append(frame_size * get_frame_count(hw_library))


def get_frame_count(hw_library):
    existing = []
    for operation in hw_library.keys():
        existing.extend(list(get_existing(hw_library[operation]["bitstreams"])))
    resource_count = dict()
    for pattern in existing:
        for char in pattern:
            resource_count[char] = resource_count.get(char, 0) + 1
    resource_cost = {"M": 216, "D": 200, "B": 196}
    frame_count = 0
    for resource in resource_count.keys():
        frame_count += resource_count[resource] * resource_cost[resource]
    return frame_count


if __name__ == '__main__':
    main(sys.argv[1:])
