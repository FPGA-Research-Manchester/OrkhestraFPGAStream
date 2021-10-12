from dataclasses import dataclass
from typing import Tuple
from sys import maxsize


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
            new_available_nodes.extend(
                get_new_available_nodes(node, new_past_nodes, all_nodes))

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


def get_module_index(start_location_index, taken_columns):
    if not taken_columns:
        raise ValueError("Taken positions can't be empty!")
    if start_location_index < taken_columns[0][0]:
        return 0
    for module_index in list(range(len(taken_columns)))[::-1]:
        if start_location_index > taken_columns[module_index][1]:
            return module_index + 1
    return len(taken_columns)


def find_all_available_bitstream(operation, min_position, taken_columns, hw_library):
    all_positions_and_bitstreams = []
    for start_location_index in range(len(hw_library[operation]["start_locations"]))[min_position:]:
        # This line possibly not needed
        if hw_library[operation]["start_locations"][start_location_index]:
            for bitstream_index in range(len(hw_library[operation]["start_locations"][start_location_index])):
                bitstream, final_column_index = get_bitstream_end_from_library_with_operation(bitstream_index,
                                                                                              start_location_index,
                                                                                              operation, hw_library)
                if not taken_columns:
                    all_positions_and_bitstreams.append(
                        (0, start_location_index, bitstream_index))
                else:
                    # Here need to check if beginning is after last taken and end is before next taken.
                    module_index = get_module_index(
                        start_location_index, taken_columns)
                    # If fits then append
                    if (len(taken_columns) == module_index and taken_columns[module_index - 1][
                        1] < start_location_index) or (
                            len(taken_columns) != module_index and taken_columns[module_index][
                        0] > final_column_index):
                        all_positions_and_bitstreams.append(
                            (module_index, start_location_index, bitstream_index))
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
                # Assuming there isn't a double dependency on the same node
                new_available_nodes.remove(potential_node)
    return new_available_nodes


def place_nodes_recursively_only_placement_check(available_nodes, past_nodes, all_nodes, current_run, current_plan,
                                                 all_plans, reduce_single_runs, hw_library, current_min_runs_pointer,
                                                 data_tables):
    if len(current_plan) <= current_min_runs_pointer[0]:
        if available_nodes:
            for node in available_nodes:
                min_position = get_min_position_in_current_run(
                    current_run, node, all_nodes)
                taken_columns = get_taken_columns(current_run)
                available_module_placements = get_scheduled_modules_for_node_after_pos(all_nodes, min_position, node,
                                                                                       taken_columns, hw_library)
                if available_module_placements:
                    # Place node in current run
                    for insert_at, module_placement in available_module_placements:
                        new_current_run = current_run.copy()
                        new_current_run[insert_at:insert_at] = [module_placement]
                        find_next_module_placement(all_nodes, all_plans, available_nodes, current_plan, hw_library,
                                                   module_placement, new_current_run, node, past_nodes,
                                                   reduce_single_runs, current_min_runs_pointer, data_tables)
                if not available_module_placements or not reduce_single_runs:
                    # Schedule current run and place node in next run
                    new_current_plan = current_plan.copy()
                    if current_run:
                        new_current_plan.append(tuple(current_run))
                    available_module_placements = get_scheduled_modules_for_node_after_pos(all_nodes, 0, node, [],
                                                                                           hw_library)
                    if available_module_placements:
                        for _, module_placement in available_module_placements:
                            find_next_module_placement(all_nodes, all_plans, available_nodes, new_current_plan,
                                                       hw_library,
                                                       module_placement, [module_placement], node, past_nodes,
                                                       reduce_single_runs, current_min_runs_pointer, data_tables)
                    else:
                        raise ValueError("Something went wrong!")

        else:
            # Out of nodes -> Save current plan and return
            current_plan.append(tuple(current_run))
            all_plans.append(tuple(current_plan))
            if len(current_plan) < current_min_runs_pointer[0]:
                current_min_runs_pointer[0] = len(current_plan)


def is_bitstream_utility_great_enough(bitstream, hw_library, node_cost, operation):
    bitstream_capacity = hw_library[operation]["bitstreams"][bitstream]["capacity"]
    satisfies_requirements = True
    missing_utility = []
    if len(bitstream_capacity) != len(node_cost):
        raise ValueError("Capacity parameters don't match")
    for capacity_parameter_index in range(len(bitstream_capacity)):
        missing_utility.append(node_cost[capacity_parameter_index] - bitstream_capacity[capacity_parameter_index])
        if bitstream_capacity[capacity_parameter_index] < node_cost[capacity_parameter_index]:
            satisfies_requirements = False

    if "composable" not in hw_library[operation]["decorators"]:
        satisfies_requirements = True

    return satisfies_requirements, missing_utility


def get_new_all_nodes_with_updated_costs(all_nodes, missing_utility, node, satisfies_requirements):
    new_all_nodes = all_nodes.copy()
    if not satisfies_requirements:
        new_all_nodes[node] = all_nodes[node].copy()
        new_capacity_values = []
        for capacity_parameter_index in range(len(missing_utility)):
            if missing_utility[capacity_parameter_index] <= 0:
                new_capacity_values.append(0)
            else:
                new_capacity_values.append(missing_utility[capacity_parameter_index])
        new_all_nodes[node]["capacity"] = tuple(new_capacity_values)
    else:
        del new_all_nodes[node]
    return new_all_nodes


def create_new_available_nodes_lists(all_nodes, available_nodes, past_nodes, node, satisfies_requirements):
    new_available_nodes = available_nodes.copy()
    new_past_nodes = past_nodes.copy()
    if satisfies_requirements:
        new_available_nodes.remove(node)
        new_past_nodes.append(node)
        new_available_nodes.extend(
            get_new_available_nodes(node, new_past_nodes, all_nodes))
    return new_available_nodes, new_past_nodes


# TODO: Update new_data_tables and retrun if the node can be returned or not (update all_new_nodes if yes!). Then run!
# This would be done by the operation driver
def update_node_table(all_nodes, new_all_nodes, node, bitstream_capacity, operation, data_tables, new_data_tables):
    if operation == "Linear Sort":
        if len(all_nodes[node]["tables"]) != 1:
            raise ValueError("Wrong number of tables for linear sort!")
        if len(bitstream_capacity) != 1:
            raise ValueError("Wrong linear sort capacity given!")
        table_name = all_nodes[node]["tables"][0]
        current_table = data_tables[table_name].copy()
        new_sorted_sequences = []
        # Update the table to have gone through the linear sorter
        sequence_count = current_table["record_count"] // bitstream_capacity[0]
        if sequence_count == 0:
            current_table["sorted_sequences"] = (0, current_table["record_count"] - 1, 0)
        else:
            for sequence_id in range(sequence_count):
                new_sorted_sequences.append((bitstream_capacity[0] * sequence_id,
                                             bitstream_capacity[0] * (sequence_id + 1) - 1, 0))
                if sequence_id == sequence_count - 1:
                    new_sorted_sequences.append((bitstream_capacity[0] * sequence_count,
                                                 (bitstream_capacity[0] * sequence_count + current_table[
                                                     "record_count"] % bitstream_capacity[0]) - 1, 0))
            current_table["sorted_sequences"] = tuple(new_sorted_sequences)
        new_data_tables[table_name] = current_table
        # Move the table name to the correct place for next nodes. This method should also go to all other non-sorting nodes!
        add_new_table_names_to_next_nodes(all_nodes, new_all_nodes, node, table_name)

        return True
    elif operation == "Merge Sort":
        pass  # Do later as it is a bit more difficult -> Check table forwarding that the node after linear sort gets the correct table and then commit!
        return True


def add_new_table_names_to_next_nodes(all_nodes, new_all_nodes, node, table_name):
    for next_node in new_all_nodes[node]["after"]:
        new_all_nodes[next_node] = all_nodes[next_node].copy()

        # Get index
        current_node_index = get_current_node_index(new_all_nodes, next_node, node)

        # Add a new name to this table
        # Assuming it is large enough!
        new_all_nodes[next_node]["tables"][current_node_index] = table_name


def get_current_node_index(new_all_nodes, next_node, node):
    current_node_index = -1
    for potential_current_node_i in range(len(new_all_nodes[next_node]["before"])):
        if new_all_nodes[next_node]["before"][potential_current_node_i] == node:
            current_node_index = potential_current_node_i
    return current_node_index


def find_next_module_placement(all_nodes, all_plans, available_nodes, new_current_plan, hw_library, module_placement,
                               new_current_run, node, past_nodes, reduce_single_runs, current_min_runs, data_tables):
    # Check requirements and utility and update all nodes accordingly no matter if the node is removed or not.
    new_all_nodes, new_data_tables, satisfies_requirements = update_all_nodes(all_nodes, module_placement.bitstream,
                                                                              data_tables, hw_library, node,
                                                                              all_nodes[node]["capacity"],
                                                                              all_nodes[node]["operation"])

    # Set new available and past nodes if the current node can be removed
    new_available_nodes, new_past_nodes = create_new_available_nodes_lists(all_nodes,
                                                                           available_nodes,
                                                                           past_nodes, node,
                                                                           satisfies_requirements)

    # Next recursive step
    place_nodes_recursively_only_placement_check(new_available_nodes, new_past_nodes,
                                                 new_all_nodes, new_current_run.copy(), new_current_plan.copy(),
                                                 all_plans, reduce_single_runs, hw_library, current_min_runs,
                                                 new_data_tables)


def update_all_nodes(all_nodes, bitstream, data_tables, hw_library, node, node_cost, operation):
    bitstream_capacity = hw_library[operation]["bitstreams"][bitstream]["capacity"]
    missing_utility = []
    new_all_nodes = all_nodes.copy()
    new_data_tables = data_tables.copy()
    if "sorting" in hw_library[operation]["decorators"]:
        # Update table
        should_node_be_removed = update_node_table(all_nodes, new_all_nodes, node, bitstream_capacity, operation,
                                                   data_tables, new_data_tables)
    else:
        # Find missing utility
        should_node_be_removed = find_missing_utility(bitstream_capacity, missing_utility, node_cost)
        update_graph_capacities(all_nodes, missing_utility, new_all_nodes, node, should_node_be_removed)
        # TODO: This needs to be done for all module drivers
        add_new_table_names_to_next_nodes(all_nodes, new_all_nodes, node, all_nodes[node]["tables"][0])
    # Update all nodes
    if should_node_be_removed:
        del new_all_nodes[node]
    return new_all_nodes, new_data_tables, should_node_be_removed


def update_graph_capacities(all_nodes, missing_utility, new_all_nodes, node, should_node_be_removed):
    if not should_node_be_removed:
        new_all_nodes[node] = all_nodes[node].copy()
        new_capacity_values = []
        for capacity_parameter_index in range(len(missing_utility)):
            if missing_utility[capacity_parameter_index] <= 0:
                new_capacity_values.append(0)
            else:
                new_capacity_values.append(missing_utility[capacity_parameter_index])
        new_all_nodes[node]["capacity"] = tuple(new_capacity_values)


def find_missing_utility(bitstream_capacity, missing_utility, node_cost):
    should_node_be_removed = True
    if len(bitstream_capacity) != len(node_cost):
        raise ValueError("Capacity parameters don't match")
    for capacity_parameter_index in range(len(bitstream_capacity)):
        missing_utility.append(node_cost[capacity_parameter_index] - bitstream_capacity[capacity_parameter_index])
        if bitstream_capacity[capacity_parameter_index] < node_cost[capacity_parameter_index]:
            should_node_be_removed = False
    return should_node_be_removed


def get_scheduled_modules_for_node_after_pos(all_nodes, min_position, node, taken_columns, hw_library):
    current_operation = all_nodes[node]["operation"]
    available_bitstreams = find_all_available_bitstream(
        current_operation, min_position, taken_columns, hw_library)
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
        "length"] - 1
    return chosen_bitstream, end_index


def print_resource_string(current_resource_string, module_coordinates):
    colour_char_len = len(FancyText.GREEN)
    for coordinate_i in range(len(module_coordinates)):
        coordinate = module_coordinates[coordinate_i]
        resulting_string = current_resource_string[:coordinate[0] + coordinate_i * 2 * colour_char_len] \
                           + FancyText.GREEN + current_resource_string[
                                               coordinate[0] + coordinate_i * 2 * colour_char_len:]
        current_resource_string = resulting_string
        resulting_string = current_resource_string[:(coordinate[1] + 1) + (coordinate_i * 2 + 1) * colour_char_len] \
                           + FancyText.DEFAULT + current_resource_string[
                                                 (coordinate[1] + 1) + (coordinate_i * 2 + 1) * colour_char_len:]
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
    place_nodes_recursively_no_placement_check(
        available_nodes, [], all_nodes, [], [], simple_plans)
    # Remove duplicates
    simple_plans = list(dict.fromkeys(simple_plans))
    # Print plans
    for plan_i in range(len(simple_plans)):
        print(f"{plan_i}: {simple_plans[plan_i]}")


def find_plans_and_print(starting_nodes, graph, resource_string, hw_library, data_tables):
    print_node_placement_permutations(starting_nodes, graph)
    print(f"{FancyText.UNDERLINE}Plans with placed modules below:{FancyText.END}")
    # Now with string match checks
    resulting_plans = []
    min_runs_pointer = [maxsize]
    place_nodes_recursively_only_placement_check(starting_nodes, [], graph, [], [], resulting_plans, True,
                                                 hw_library, min_runs_pointer, data_tables)
    resulting_plans = list(dict.fromkeys(resulting_plans))
    print_statistics(resulting_plans)
    print_all_runs_using_less_runs_than(resulting_plans, min_runs_pointer[0] + 1, resource_string)


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
                                ['filter162.bit', 'filter321.bit'], [
                                    'filter81.bit'], [], [], [], ['filter324.bit'],
                                ['filter84.bit', 'filter322.bit'], [], [
                                    'filter164.bit'], [],
                                ['filter162.bit', 'filter321.bit'], [
                                    'filter81.bit'], [], [], [], ['filter324.bit'],
                                ['filter84.bit', 'filter322.bit'], [], [
                                    'filter164.bit'], [],
                                ['filter162.bit', 'filter321.bit'], ['filter81.bit'], [], [], []],
            "decorators": ["composable"]
        }, "Linear Sort": {
            "bitstreams": {
                "linear512.bit": {"locations": (1, 11, 21), "length": 10, "capacity": (512,), "string": "BDMMBDMDMM",
                                  "is_backwards": False},
                "linear1024.bit": {"locations": (1, 11), "length": 14, "capacity": (1024,), "string": "BDMMBDMDMMBDMM",
                                   "is_backwards": False}},
            "start_locations": [[], ['linear512.bit', 'linear1024.bit'], [], [], [], [], [], [], [], [], [],
                                ['linear512.bit', 'linear1024.bit'], [
                                ], [], [], [], [], [], [], [], [],
                                ['linear512.bit'],
                                [], [], [], [], [], [], [], [], []],
            "decorators": ["sorting"]
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
                                [], [], [], ['mergesort64.bit'], [], [], ['mergesort32.bit'], [], [], [], [], [], []],
            "decorators": ["composable", "sorting", "blocking"]
        }, "Merge Join": {
            "bitstreams": {
                "join.bit": {"locations": (5, 15, 25), "length": 6, "capacity": (), "string": "BDMDMM",
                             "is_backwards": False}},
            "start_locations": [[], [], [], [], [], ['join.bit'], [], [], [], [], [], [], [], [], [], ['join.bit'], [],
                                [],
                                [], [], [], [], [], [], [], ['join.bit'], [], [], [], [], []],
            "decorators": []
        }, "Addition": {
            "bitstreams": {
                "addition.bit": {"locations": (7, 17, 27), "length": 4, "capacity": (), "string": "MDMM",
                                 "is_backwards": False}},
            "start_locations": [[], [], [], [], [], [], [], ['addition.bit'], [], [], [], [], [], [], [], [], [],
                                ['addition.bit'], [], [], [], [], [], [], [], [], [], ['addition.bit'], [], [], []],
            "decorators": []
        }, "Multiplier": {
            "bitstreams": {
                "multiplier.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (), "string": "MBDMMBD",
                                   "is_backwards": False}},
            "start_locations": [['multiplier.bit'], [], [], [], [], [], [], [], [], [], ['multiplier.bit'], [], [], [],
                                [],
                                [], [], [], [], [], ['multiplier.bit'], [], [], [], [], [], [], [], [], [], []],
            "decorators": []
        }, "Global Sum": {
            "bitstreams": {
                "globalsum.bit": {"locations": (6, 16, 26), "length": 3, "capacity": (), "string": "DMD",
                                  "is_backwards": False}},
            "start_locations": [[], [], [], [], [], [], ['globalsum.bit'], [], [], [], [], [], [], [], [], [],
                                ['globalsum.bit'], [], [], [], [], [], [], [
                                ], [], [], ['globalsum.bit'], [], [], [],
                                []],
            "decorators": []
        }
    }

    graph = {
        "A": {"operation": "Addition", "capacity": (), "before": (), "after": ("B",),
              "tables": ["test_table"]},
        "B": {"operation": "Multiplier", "capacity": (), "before": ("A",), "after": (), "tables": [""]},
        "C": {"operation": "Global Sum", "capacity": (), "before": (), "after": (), "tables": ["test_table2"]}
    }
    q19_graph = {
        "FirstFilter": {"operation": "Filter", "capacity": (4, 2), "before": ("test_table",), "after": ("LinSort",),
                        "tables": ["test_table"]},
        "LinSort": {"operation": "Linear Sort", "capacity": (10), "before": ("FirstFilter",),
                    "after": ("MergeSort",), "tables": [""]},
        "MergeSort": {"operation": "Merge Sort", "capacity": (10), "before": ("LinSort",), "after": ("Join",),
                      "tables": [""]},
        "Join": {"operation": "Merge Join", "capacity": (), "before": ("MergeSort",), "after": ("SecondFilter",),
                 "tables": [""]},
        "SecondFilter": {"operation": "Filter", "capacity": (12, 4), "before": ("Join",), "after": ("Add",),
                         "tables": [""]},
        "Add": {"operation": "Addition", "capacity": (), "before": ("SecondFilter",), "after": ("Mul",),
                "tables": [""]},
        "Mul": {"operation": "Multiplier", "capacity": (), "before": ("Add",), "after": ("Sum",), "tables": [""]},
        "Sum": {"operation": "Global Sum", "capacity": (), "before": ("Mul",), "after": (), "tables": [""]}
    }
    capacity_test = {
        "FirstFilter": {"operation": "Filter", "capacity": (4, 2), "before": (), "after": ("LinSort",),
                        "tables": ["test_table2"]},
        "LinSort": {"operation": "Linear Sort", "capacity": (10), "before": ("FirstFilter",), "after": ("Sum",),
                    "tables": [""]},
        "Sum": {"operation": "Global Sum", "capacity": (), "before": ("LinSort",), "after": (), "tables": [""]}
    }
    # Sorted sequence = ((begin, end, column_i),...); empty = unsorted
    tables = {
        "test_table": {"record_count": 10, "sorted_sequences": ()},
        "test_table2": {"record_count": 10000, "sorted_sequences": ((0, 9999, 0),)}
    }

    # starting_nodes = ["A", "C"]
    # find_plans_and_print(starting_nodes, graph, resource_string, hw_library, tables)
    capacity_test_nodes = ["FirstFilter"]
    find_plans_and_print(capacity_test_nodes, capacity_test,
                         resource_string, hw_library, tables)
    # q19_starting_nodes = ["FirstFilter"]
    # find_plans_and_print(q19_starting_nodes, q19_graph,
    #                      resource_string, hw_library, tables)


if __name__ == '__main__':
    main()
    # Missing parts:
    # 1. Correct stream capacity
    # 2. Backwards paths
    # Decorators:
    #   Composable
    #   First module
    #   Reducing
    #   Increasing
    #   How many outputs - Just the graph has to say 1st or 2nd output
    # Optimisation rules:
    #   Merge sort removal
    #   Reordering
    #   Data duplication for filters
