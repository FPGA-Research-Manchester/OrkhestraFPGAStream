from dataclasses import dataclass
from typing import Tuple


@dataclass(eq=True, frozen=True)
class ScheduledModule:
    node_name: str
    bitstream: str
    position: Tuple[int, int]


class FancyText:
    PURPLE = '\033[95m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    DEFAULT = '\033[39m'
    END = '\033[0m'


# DON'T TOUCH THIS ONE!
def place_nodes_recursively_no_placement_check(available_nodes, past_nodes, all_nodes, current_run, current_plan,
                                               all_plans):
    if available_nodes:
        for node in available_nodes:
            # Update available nodes copy for next recursive calls
            new_available_nodes = available_nodes.copy()
            new_available_nodes.remove(node)
            new_past_nodes = past_nodes.copy()
            new_past_nodes.append(node)
            new_available_nodes.extend(get_new_available_nodes(node, new_past_nodes, all_nodes))

            # Place node in current run
            new_current_run = current_run.copy()
            new_current_run.append(node)
            place_nodes_recursively_no_placement_check(new_available_nodes.copy(), new_past_nodes, all_nodes,
                                                       new_current_run,
                                                       current_plan.copy(), all_plans)

            # Schedule current run and place node in next run
            new_current_plan = current_plan.copy()
            if current_run:
                new_current_plan.append(tuple(current_run))
            place_nodes_recursively_no_placement_check(new_available_nodes.copy(), new_past_nodes, all_nodes, [node],
                                                       new_current_plan,
                                                       all_plans)
    else:
        # Out of nodes -> Save current plan and return
        current_plan.append(tuple(current_run))
        all_plans.append(tuple(current_plan))
        return


# Old
# def suitable_for_current_run_returns_false():
#     return False
#
#
# # Return only first available
# def find_first_available_bitstream(operation):
#     for start_location_index in range(len(hw_library[operation]["start_locations"])):
#         if hw_library[operation]["start_locations"][start_location_index]:
#             return start_location_index, 0
#
#
#
# def place_nodes_recursively_only_placement_check(available_nodes, all_nodes, current_run, current_plan, all_plans):
#     if available_nodes:
#         for node in available_nodes:
#             # Update available nodes copy for next recursive calls
#             new_available_nodes = available_nodes.copy()
#             new_available_nodes.remove(node)
#             new_available_nodes.extend(all_nodes[node]["after"])
#
#             # Always false for now
#             if suitable_for_current_run_returns_false():
#                 # Place node in current run
#                 new_current_run = current_run.copy()
#                 new_current_run.append(node)
#                 place_nodes_recursively_only_placement_check(new_available_nodes.copy(), all_nodes, new_current_run,
#                                                              current_plan.copy(), all_plans)
#
#             # Schedule current run and place node in next run
#             new_current_plan = current_plan.copy()
#             if current_run:
#                 new_current_plan.append(tuple(current_run))
#             current_operation = all_nodes[node]["operation"]
#             chosen_position, chosen_bitstream_index = find_first_available_bitstream(current_operation)
#             chosen_bitstream = hw_library[current_operation]["start_locations"][chosen_position]
#             [chosen_bitstream_index]
#             end_index = chosen_position + hw_library[current_operation]["bitstreams"][chosen_bitstream]["length"]
#             new_current_run = [ScheduledModule(node, chosen_bitstream, (chosen_position, end_index))]
#             place_nodes_recursively_only_placement_check(new_available_nodes.copy(), all_nodes, new_current_run,
#                                                          new_current_plan, all_plans)
#     else:
#         # Out of nodes -> Save current plan and return
#         current_plan.append(tuple(current_run))
#         all_plans.append(tuple(current_plan))
#         return


def get_module_index(start_location_index, taken_columns):
    if not taken_columns:
        raise ValueError("Taken positions can't be empty!")
    if start_location_index < taken_columns[0][0]:
        return 0
    for module_index in range(len(taken_columns)):
        if start_location_index > taken_columns[module_index][1]:
            return module_index + 1
    return len(taken_columns)


def find_all_available_bitstream(operation, min_position, taken_columns, hw_library):
    all_positions_and_bitstreams = []
    for start_location_index in range(len(hw_library[operation]["start_locations"]))[min_position:]:
        if hw_library[operation]["start_locations"][start_location_index]:  # This line possibly not needed
            for bitstream_index in range(len(hw_library[operation]["start_locations"][start_location_index])):
                if not taken_columns:
                    all_positions_and_bitstreams.append((0, start_location_index, bitstream_index))
                else:
                    # Here need to check if beginning is after last taken and end is before next taken.
                    module_index = get_module_index(start_location_index, taken_columns)
                    _, final_column_index = get_bitstream_end_from_library_with_operation(bitstream_index,
                                                                                          start_location_index,
                                                                                          operation, hw_library)
                    # If fits then append
                    if (len(taken_columns) == module_index and taken_columns[module_index - 1][
                        1] < start_location_index) or (
                            len(taken_columns) != module_index and taken_columns[module_index][0] > final_column_index):
                        all_positions_and_bitstreams.append((module_index, start_location_index, bitstream_index))
    return all_positions_and_bitstreams


def get_taken_columns(current_run):
    taken_columns = []
    for module in current_run:
        taken_columns.append(module.position)
    return taken_columns


def get_min_position_in_current_run(current_run, node, all_nodes):
    prereq_nodes = all_nodes[node]["before"]
    currently_scheduled_prereq_nodes = []
    for previous_node in prereq_nodes:
        for module in current_run:
            if previous_node == module.node_name:
                currently_scheduled_prereq_nodes.append(module)
    if currently_scheduled_prereq_nodes:
        current_min = 0
        for module in currently_scheduled_prereq_nodes:
            if module.position[1] > current_min:
                current_min = module.position[1]
        return current_min + 1
    else:
        return 0


# TODO: Multiple output nodes has to be checked if it produces 1 or N outputs
def get_new_available_nodes(scheduled_node, past_nodes, all_nodes):
    potential_nodes = list(all_nodes[scheduled_node]["after"])
    new_available_nodes = potential_nodes.copy()
    for potential_node in potential_nodes:
        for previous_node in all_nodes[potential_node]["before"]:
            if previous_node not in past_nodes:
                new_available_nodes.remove(potential_node)
    return new_available_nodes


def place_nodes_recursively_only_placement_check_v2(available_nodes, past_nodes, all_nodes, current_run, current_plan,
                                                    all_plans, reduce_single_runs, hw_library):
    if available_nodes:
        for node in available_nodes:
            # Update available nodes copy for next recursive calls
            new_available_nodes = available_nodes.copy()
            new_available_nodes.remove(node)
            new_past_nodes = past_nodes.copy()
            new_past_nodes.append(node)
            new_available_nodes.extend(get_new_available_nodes(node, new_past_nodes, all_nodes))

            min_position = get_min_position_in_current_run(current_run, node, all_nodes)
            taken_columns = get_taken_columns(current_run)
            available_module_placements = get_scheduled_modules_for_node_after_pos(all_nodes, min_position, node,
                                                                                   taken_columns, hw_library)
            if available_module_placements:
                # Place node in current run
                for insert_at, module_placement in available_module_placements:
                    new_current_run = current_run.copy()
                    new_current_run[insert_at:insert_at] = [module_placement]
                    place_nodes_recursively_only_placement_check_v2(new_available_nodes.copy(), new_past_nodes,
                                                                    all_nodes, new_current_run, current_plan.copy(),
                                                                    all_plans, reduce_single_runs, hw_library)
            if not available_module_placements or not reduce_single_runs:
                # Schedule current run and place node in next run
                new_current_plan = current_plan.copy()
                if current_run:
                    new_current_plan.append(tuple(current_run))
                available_module_placements = get_scheduled_modules_for_node_after_pos(all_nodes, 0, node, [],
                                                                                       hw_library)
                if available_module_placements:
                    for _, module_placement in available_module_placements:
                        place_nodes_recursively_only_placement_check_v2(new_available_nodes.copy(), new_past_nodes,
                                                                        all_nodes, [module_placement],
                                                                        new_current_plan.copy(), all_plans,
                                                                        reduce_single_runs, hw_library)
                else:
                    raise ValueError("Something went wrong!")

    else:
        # Out of nodes -> Save current plan and return
        current_plan.append(tuple(current_run))
        all_plans.append(tuple(current_plan))
        return


def get_scheduled_modules_for_node_after_pos(all_nodes, min_position, node, taken_columns, hw_library):
    current_operation = all_nodes[node]["operation"]
    available_bitstreams = find_all_available_bitstream(current_operation, min_position, taken_columns, hw_library)
    available_module_placements = []
    if available_bitstreams:
        for chosen_module_position, chosen_column_position, chosen_bitstream_index in available_bitstreams:
            chosen_bitstream, end_index = get_bitstream_end_from_library_with_operation(chosen_bitstream_index,
                                                                                        chosen_column_position,
                                                                                        current_operation, hw_library)
            available_module_placements.append(
                (chosen_module_position, ScheduledModule(node, chosen_bitstream, (chosen_column_position, end_index))))
    return available_module_placements


def get_bitstream_end_from_library_with_operation(chosen_bitstream_index, chosen_column_position, current_operation,
                                                  hw_library):
    chosen_bitstream = hw_library[current_operation]["start_locations"][chosen_column_position][
        chosen_bitstream_index]
    end_index = chosen_column_position + hw_library[current_operation]["bitstreams"][chosen_bitstream][
        "length"]
    return chosen_bitstream, end_index


def print_resource_string(current_resource_string, module_coordinates):
    colour_char_len = len(FancyText.GREEN)
    for coordinate_i in range(len(module_coordinates)):
        coordinate = module_coordinates[coordinate_i]
        resulting_string = current_resource_string[:coordinate[0] + coordinate_i * 2 * colour_char_len] \
                           + FancyText.GREEN + current_resource_string[
                                               coordinate[0] + coordinate_i * 2 * colour_char_len:]
        current_resource_string = resulting_string
        resulting_string = current_resource_string[:coordinate[1] + (coordinate_i * 2 + 1) * colour_char_len] \
                           + FancyText.DEFAULT + current_resource_string[
                                                 coordinate[1] + (coordinate_i * 2 + 1) * colour_char_len:]
        current_resource_string = resulting_string
    print(current_resource_string)


def print_statistics(plans):
    print(f"Plan count:{len(plans)}")
    run_counts = {}
    for plan in plans:
        run_count = len(plan)
        if run_count in run_counts:
            run_counts[run_count] += 1
        else:
            run_counts[run_count] = 1
    for run_count, occurrences in sorted(run_counts.items()):
        print(f"{run_count} run plans:{occurrences}")


def print_all_runs_using_less_runs_than(plans, run_limit, resource_string):
    for plan_i in range(len(plans)):
        if len(plans[plan_i]) < run_limit:
            print()
            print(f"Plan {plan_i}:")
            for run_i in range(len(plans[plan_i])):
                print(f"Run#{run_i}:")
                node_names = []
                bitstream_names = []
                start_and_ends = []
                for module in plans[plan_i][run_i]:
                    node_names.append(module.node_name)
                    bitstream_names.append(module.bitstream)
                    start_and_ends.append(module.position)
                print(node_names)
                print(bitstream_names)
                print_resource_string(resource_string, start_and_ends)


def print_node_placement_permutations(available_nodes, all_nodes):
    simple_plans = []
    # For each choice:
    #   Make the choice and then update available nodes and come back to initial step
    place_nodes_recursively_no_placement_check(available_nodes, [], all_nodes, [], [], simple_plans)
    # Remove duplicates
    simple_plans = list(dict.fromkeys(simple_plans))
    # Print plans
    for plan_i in range(len(simple_plans)):
        print(f"{plan_i}: {simple_plans[plan_i]}")


def find_plans_and_print(starting_nodes, graph, resource_string, hw_library):
    print_node_placement_permutations(starting_nodes, graph)
    print(f"{FancyText.UNDERLINE}Plans with placed modules below:{FancyText.END}")
    # Now with string match checks
    resulting_plans = []
    place_nodes_recursively_only_placement_check_v2(starting_nodes, [], graph, [], [], resulting_plans, False,
                                                    hw_library)
    resulting_plans = list(dict.fromkeys(resulting_plans))
    print_statistics(resulting_plans)
    print_all_runs_using_less_runs_than(resulting_plans, 2, resource_string)


def main():
    resource_string = "MBDMMBDMDMMBDMMBDMDMMBDMMBDMDMM"
    hw_library = {
        "Filter": {
            "bitstreams": {
                "filter81.bit": {"locations": (7, 17, 27), "length": 4, "capacity": (8, 1), "string": "MDMM",
                                 "is_backwards": False},
                "filter84.bit": {"locations": (2, 12, 22), "length": 7, "capacity": (8, 4), "string": "DMMBDMD",
                                 "is_backwards": False},
                "filter162.bit": {"locations": (6, 16, 26), "length": 5, "capacity": (16, 2), "string": "DMDMM",
                                  "is_backwards": False},
                "filter164.bit": {"locations": (4, 14, 24), "length": 7, "capacity": (16, 4), "string": "MBDMDMM",
                                  "is_backwards": False},
                "filter321.bit": {"locations": (6, 16, 26), "length": 5, "capacity": (32, 1), "string": "DMDMM",
                                  "is_backwards": False},
                "filter322.bit": {"locations": (2, 12, 22), "length": 7, "capacity": (32, 2), "string": "DMMBDMD",
                                  "is_backwards": False},
                "filter324.bit": {"locations": (1, 11, 21), "length": 10, "capacity": (32, 4), "string": "BDMMBDMDMM",
                                  "is_backwards": False}},
            "start_locations": [[], ['filter324.bit'], ['filter84.bit', 'filter322.bit'], [], ['filter164.bit'], [],
                                ['filter162.bit', 'filter321.bit'], ['filter81.bit'], [], [], [], ['filter324.bit'],
                                ['filter84.bit', 'filter322.bit'], [], ['filter164.bit'], [],
                                ['filter162.bit', 'filter321.bit'], ['filter81.bit'], [], [], [], ['filter324.bit'],
                                ['filter84.bit', 'filter322.bit'], [], ['filter164.bit'], [],
                                ['filter162.bit', 'filter321.bit'], ['filter81.bit'], [], [], []]
        }, "Linear Sort": {
            "bitstreams": {
                "linear512.bit": {"locations": (1, 11, 21), "length": 10, "capacity": (512,), "string": "BDMMBDMDMM",
                                  "is_backwards": False},
                "linear1024.bit": {"locations": (1, 11), "length": 14, "capacity": (1024,), "string": "BDMMBDMDMMBDMM",
                                   "is_backwards": False}},
            "start_locations": [[], ['linear512.bit', 'linear1024.bit'], [], [], [], [], [], [], [], [], [],
                                ['linear512.bit', 'linear1024.bit'], [], [], [], [], [], [], [], [], [],
                                ['linear512.bit'],
                                [], [], [], [], [], [], [], [], []]
        }, "Merge Sort": {
            "bitstreams": {
                "mergesort32.bit": {"locations": (4, 14, 24), "length": 7, "capacity": (32,), "string": "MBDMDMM",
                                    "is_backwards": False},
                "mergesort64.bit": {"locations": (1, 11, 21), "length": 10, "capacity": (64,), "string": "BDMMBDMDMM",
                                    "is_backwards": False},
                "mergesort128.bit": {"locations": (1, 11), "length": 12, "capacity": (128,), "string": "BDMMBDMDMMBD",
                                     "is_backwards": False}},
            "start_locations": [[], ['mergesort64.bit', 'mergesort128.bit'], [], [], ['mergesort32.bit'], [], [], [],
                                [],
                                [], [], ['mergesort64.bit', 'mergesort128.bit'], [], [], ['mergesort32.bit'], [], [],
                                [],
                                [], [], [], ['mergesort64.bit'], [], [], ['mergesort32.bit'], [], [], [], [], [], []]
        }, "Merge Join": {
            "bitstreams": {
                "join.bit": {"locations": (5, 15, 25), "length": 6, "capacity": (), "string": "BDMDMM",
                             "is_backwards": False}},
            "start_locations": [[], [], [], [], [], ['join.bit'], [], [], [], [], [], [], [], [], [], ['join.bit'], [],
                                [],
                                [], [], [], [], [], [], [], ['join.bit'], [], [], [], [], []]
        }, "Addition": {
            "bitstreams": {
                "addition.bit": {"locations": (7, 17, 27), "length": 4, "capacity": (), "string": "MDMM",
                                 "is_backwards": False}},
            "start_locations": [[], [], [], [], [], [], [], ['addition.bit'], [], [], [], [], [], [], [], [], [],
                                ['addition.bit'], [], [], [], [], [], [], [], [], [], ['addition.bit'], [], [], []]
        }, "Multiplier": {
            "bitstreams": {
                "multiplier.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (), "string": "MBDMMBD",
                                   "is_backwards": False}},
            "start_locations": [['multiplier.bit'], [], [], [], [], [], [], [], [], [], ['multiplier.bit'], [], [], [],
                                [],
                                [], [], [], [], [], ['multiplier.bit'], [], [], [], [], [], [], [], [], [], []]
        }, "Global Sum": {
            "bitstreams": {
                "globalsum.bit": {"locations": (6, 16, 26), "length": 3, "capacity": (), "string": "DMD",
                                  "is_backwards": False}},
            "start_locations": [[], [], [], [], [], [], ['globalsum.bit'], [], [], [], [], [], [], [], [], [],
                                ['globalsum.bit'], [], [], [], [], [], [], [], [], [], ['globalsum.bit'], [], [], [],
                                []]
        }
    }

    graph = {
        "A": {"operation": "Addition", "capacity": (), "before": (), "after": ("B",)},
        "B": {"operation": "Multiplier", "capacity": (), "before": ("A",), "after": ()},
        "C": {"operation": "Global Sum", "capacity": (), "before": (), "after": ()}
    }
    starting_nodes = ["A", "C"]
    find_plans_and_print(starting_nodes, graph, resource_string, hw_library)


if __name__ == '__main__':
    main()
    # Missing parts:
    # 1. Capacity checks
    # 2. Backwards paths
    # Decorators:
    #   Composable
    #   First module
    #   Reducing
    #   Increasing
    #   How many outputs Bigger problem!
    # Optimisation rules:
    #   Merge sort removal
    #   Reordering
    #   Data duplication for filters
