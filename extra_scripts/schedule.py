from dataclasses import dataclass
from typing import Tuple
from sys import maxsize
from enum import Enum
from time import perf_counter


@dataclass(eq=True, frozen=True)
class ScheduledModule:
    node_name: str
    operation: str
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


def select_all_placements_enum_func(available_module_placements):
    return available_module_placements


def select_first_placement_enum_func(available_module_placements):
    chosen_module_placements = []
    min_available_position = maxsize
    for placement in available_module_placements:
        if placement[1].position[0] < min_available_position:
            min_available_position = placement[1].position[0]
    for placement in available_module_placements:
        if placement[1].position[0] == min_available_position and placement not in chosen_module_placements:
            chosen_module_placements.append(placement)
    return chosen_module_placements


def select_last_placement_enum_func(available_module_placements):
    chosen_module_placements = []
    max_available_position = 0
    for placement in available_module_placements:
        if placement[1].position[0] > max_available_position:
            max_available_position = placement[1].position[0]
    for placement in available_module_placements:
        if placement[1].position[0] == max_available_position and placement not in chosen_module_placements:
            chosen_module_placements.append(placement)
    return chosen_module_placements


def select_shortest_placement_enum_func(available_module_placements):
    chosen_module_placements = []
    min_module_size = maxsize
    for placement in available_module_placements:
        if placement[1].position[1] - placement[1].position[0] + 1 < min_module_size:
            min_module_size = placement[1].position[1] - \
                              placement[1].position[0] + 1
    for placement in available_module_placements:
        if placement[1].position[1] - placement[1].position[0] + 1 == min_module_size \
                and placement not in chosen_module_placements:
            chosen_module_placements.append(placement)
    return chosen_module_placements


def select_longest_placement_enum_func(available_module_placements):
    chosen_module_placements = []
    max_module_size = 0
    for placement in available_module_placements:
        if placement[1].position[1] - placement[1].position[0] + 1 > max_module_size:
            max_module_size = placement[1].position[1] - \
                              placement[1].position[0] + 1
    for placement in available_module_placements:
        if placement[1].position[1] - placement[1].position[0] + 1 == max_module_size \
                and placement not in chosen_module_placements:
            chosen_module_placements.append(placement)
    return chosen_module_placements


class ModuleSelection(Enum):
    ALL_AVAILABLE = (select_all_placements_enum_func,)
    FIRST_AVAILABLE = (select_first_placement_enum_func,)
    LAST_AVAILABLE = (select_last_placement_enum_func,)
    SHORTEST = (select_shortest_placement_enum_func,)
    LONGEST = (select_longest_placement_enum_func,)

    def __call__(self, *args, **kwargs):
        return self.value[0](*args, **kwargs)


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


def current_run_has_first_module(hw_library, current_run, node):
    for scheduled_module in current_run:
        if scheduled_module.node_name != node \
                and "first_module" in hw_library[scheduled_module.operation]["decorators"]:
            return True
    return False


def get_min_position_in_current_run(current_run, node, all_nodes):
    prereq_nodes = all_nodes[node]["before"]
    currently_scheduled_prereq_nodes = []
    for previous_node, _ in prereq_nodes:
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


def get_new_available_nodes(scheduled_node, past_nodes, all_nodes):
    potential_nodes = list(all_nodes[scheduled_node]["after"])
    new_available_nodes = potential_nodes.copy()
    for potential_node in potential_nodes:
        for previous_node, _ in all_nodes[potential_node]["before"]:
            if previous_node not in past_nodes and previous_node in new_available_nodes:
                new_available_nodes.remove(potential_node)
    return new_available_nodes


def remove_unavailable_nodes_in_this_run(available_nodes, current_run, hw_library, all_nodes, first_nodes):
    available_nodes_for_this_run = available_nodes.copy()
    for node in available_nodes:
        if node in first_nodes:
            for module in current_run:
                for before_node_name, _ in all_nodes[node]["before"]:
                    if module.node_name == before_node_name and node in available_nodes_for_this_run:
                        available_nodes_for_this_run.remove(node)
        operation = all_nodes[node]["operation"]
        if "first_module" in hw_library[operation]["decorators"] \
                and current_run_has_first_module(hw_library, current_run,
                                                 node) and node in available_nodes_for_this_run:
            available_nodes_for_this_run.remove(node)
    return available_nodes_for_this_run


# Main recursive loop
def place_nodes_recursively(available_nodes, past_nodes, all_nodes, current_run, current_plan,
                            all_plans, reduce_single_runs, hw_library, current_min_runs_pointer,
                            data_tables, module_placement_selections, skipped_placements, first_nodes):
    if len(current_plan) <= current_min_runs_pointer[0]:
        if available_nodes:
            available_nodes_in_this_run = remove_unavailable_nodes_in_this_run(
                available_nodes, current_run, hw_library, all_nodes, first_nodes)
            for node in available_nodes:
                available_module_placements = []
                if node in available_nodes_in_this_run:
                    min_position = get_min_position_in_current_run(current_run, node, all_nodes)
                    taken_columns = get_taken_columns(current_run)
                    available_module_placements = get_scheduled_modules_for_node_after_pos(all_nodes, min_position,
                                                                                           node,
                                                                                           taken_columns, hw_library,
                                                                                           module_placement_selections,
                                                                                           skipped_placements)
                    if available_module_placements:
                        # Place node in current run
                        for insert_at, module_placement in available_module_placements:
                            new_current_run = current_run.copy()
                            new_current_run[insert_at:insert_at] = [
                                module_placement]
                            find_next_module_placement(all_nodes, all_plans, available_nodes, current_plan, hw_library,
                                                       module_placement, new_current_run, node, past_nodes,
                                                       reduce_single_runs, current_min_runs_pointer, data_tables,
                                                       module_placement_selections, skipped_placements, first_nodes)
                if not available_module_placements or not reduce_single_runs:
                    # Schedule current run and place node in next run
                    new_current_plan = current_plan.copy()
                    if current_run:
                        new_current_plan.append(tuple(current_run))
                    available_module_placements = get_scheduled_modules_for_node_after_pos(all_nodes, 0, node, [],
                                                                                           hw_library,
                                                                                           module_placement_selections,
                                                                                           skipped_placements)
                    if available_module_placements:
                        for _, module_placement in available_module_placements:
                            find_next_module_placement(all_nodes, all_plans, available_nodes, new_current_plan,
                                                       hw_library,
                                                       module_placement, [
                                                           module_placement], node, past_nodes,
                                                       reduce_single_runs, current_min_runs_pointer, data_tables,
                                                       module_placement_selections, skipped_placements, first_nodes)
                    else:
                        raise ValueError("Something went wrong!")

        else:
            # Out of nodes -> Save current plan and return
            current_plan.append(tuple(current_run))
            all_plans.append(tuple(current_plan))
            if len(current_plan) < current_min_runs_pointer[0]:
                current_min_runs_pointer[0] = len(current_plan)


def create_new_available_nodes_lists(all_nodes, available_nodes, past_nodes, node, satisfies_requirements):
    new_available_nodes = available_nodes.copy()
    new_past_nodes = past_nodes.copy()
    if satisfies_requirements:
        new_available_nodes.remove(node)
        new_past_nodes.append(node)
        new_available_nodes.extend(
            get_new_available_nodes(node, new_past_nodes, all_nodes))
    return new_available_nodes, new_past_nodes


def is_table_sorted(table, column_id):
    return len(table["sorted_sequences"]) == 1 and table["sorted_sequences"][0][0] == 0 and \
           table["sorted_sequences"][0][1] == table["record_count"] and table["sorted_sequences"][0][2] == column_id


# This would be done by the operation driver
def update_node_data_tables(all_nodes, node, bitstream_capacity, current_node_decorators, data_tables, new_data_tables):
    if "partial_sort" in current_node_decorators:
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
            current_table["sorted_sequences"] = (
                (0, current_table["record_count"], 0),)
        else:
            for sequence_id in range(sequence_count):
                new_sorted_sequences.append(
                    (bitstream_capacity[0] * sequence_id, bitstream_capacity[0], 0))
                if sequence_id == sequence_count - 1:
                    new_sorted_sequences.append((bitstream_capacity[0] * sequence_count, current_table[
                        "record_count"] % bitstream_capacity[0], 0))
            current_table["sorted_sequences"] = tuple(new_sorted_sequences)
        new_data_tables[table_name] = current_table
        return True
    elif "blocking_sort" in current_node_decorators:
        if len(all_nodes[node]["tables"]) != 1:
            raise ValueError("Wrong number of tables for merge sort!")
        if len(bitstream_capacity) != 1:
            raise ValueError("Wrong merge sort capacity given!")
        table_name = all_nodes[node]["tables"][0]
        current_table = data_tables[table_name].copy()
        # Not checking if it is sorted with the correct column_id (just id 0)
        if is_table_sorted(current_table, 0):
            raise ValueError("Table is sorted already!")
        new_sorted_sequences = []
        current_sequences = current_table["sorted_sequences"]
        if len(current_sequences) == 0:
            new_sorted_sequences.append(
                (0, min(bitstream_capacity[0], current_table["record_count"]), 0))
        else:
            new_sequence_length = 0
            for i in range(min(bitstream_capacity[0], len(current_sequences))):
                new_sequence_length += current_sequences[i][1]
            if bitstream_capacity[0] > len(current_sequences) and new_sequence_length < current_table["record_count"]:
                new_sequence_length += bitstream_capacity[0] - len(
                    current_sequences)
                new_sequence_length = min(
                    new_sequence_length, current_table["record_count"])
            new_sorted_sequences.append((0, new_sequence_length, 0))
            if new_sequence_length < current_table["record_count"] and bitstream_capacity[0] < len(current_sequences):
                for sequence_i in range(bitstream_capacity[0], len(current_sequences)):
                    new_sorted_sequences.append(current_sequences[sequence_i])
        current_table["sorted_sequences"] = tuple(new_sorted_sequences)
        new_data_tables[table_name] = current_table
        if is_table_sorted(current_table, 0):
            return True
        else:
            return False


def add_new_table_to_next_nodes(all_nodes, new_all_nodes, node, table_names):
    for next_node in new_all_nodes[node]["after"]:
        new_all_nodes[next_node] = all_nodes[next_node].copy()

        # Get index
        current_node_indexes = get_current_node_index(
            new_all_nodes, next_node, node)

        for node_table_index, current_node_stream_index in current_node_indexes:
            # Add a new name to this table
            # Assuming it is large enough!
            new_all_nodes[next_node]["tables"][node_table_index] = table_names[current_node_stream_index]


def get_current_node_index(new_all_nodes, next_node, node_name):
    current_node_indexes = []
    for potential_current_node_i in range(len(new_all_nodes[next_node]["before"])):
        if new_all_nodes[next_node]["before"][potential_current_node_i][0] == node_name:
            current_node_indexes.append(
                (potential_current_node_i, new_all_nodes[next_node]["before"][potential_current_node_i][1]))
    if len(current_node_indexes) == 0:
        raise ValueError("No next nodes found with the expected dependencies!")
    return current_node_indexes


def find_next_module_placement(all_nodes, all_plans, available_nodes, new_current_plan, hw_library, module_placement,
                               new_current_run, node, past_nodes, reduce_single_runs, current_min_runs, data_tables,
                               module_placement_selections, skipped_placements, first_nodes):
    # Check requirements and utility and update all nodes accordingly no matter if the node is removed or not.
    new_all_nodes, new_data_tables, satisfies_requirements, skipped_nodes = update_all_nodes(all_nodes,
                                                                                             module_placement.bitstream,
                                                                                             data_tables, hw_library,
                                                                                             node,
                                                                                             all_nodes[node][
                                                                                                 "capacity"],
                                                                                             all_nodes[node][
                                                                                                 "operation"])
    # Set new available and past nodes if the current node can be removed
    new_available_nodes, new_past_nodes = create_new_available_nodes_lists(all_nodes,
                                                                           available_nodes,
                                                                           past_nodes, node,
                                                                           satisfies_requirements)
    for skipped_node in skipped_nodes:
        if skipped_node not in new_available_nodes:
            raise ValueError("Skipped nodes processes in the wrong order!")
        else:
            new_available_nodes, new_past_nodes = create_new_available_nodes_lists(all_nodes,
                                                                                   new_available_nodes,
                                                                                   new_past_nodes, skipped_node,
                                                                                   satisfies_requirements)

    # Next recursive step
    place_nodes_recursively(new_available_nodes, new_past_nodes,
                            new_all_nodes, new_current_run.copy(), new_current_plan.copy(),
                            all_plans, reduce_single_runs, hw_library, current_min_runs,
                            new_data_tables, module_placement_selections, skipped_placements, first_nodes)


def get_resulting_tables(input_tables, current_node_decorators, new_data_tables):
    if len(input_tables) == 0:
        # Data generator decorators haven't been added yet!
        raise ValueError("No input data found!")
    if "sorted_input" in current_node_decorators:
        for table_name in input_tables:
            if not is_table_sorted(new_data_tables[table_name], 0):
                raise ValueError("Table should be sorted!")
    # Simplified decorator for equi-join
    if "largest_input_is_output" in current_node_decorators:
        max_table_name = ""
        max_size = 0
        for table_name in input_tables:
            if new_data_tables[table_name]["record_count"] >= max_size:
                max_table_name = table_name
                max_size = new_data_tables[table_name]["record_count"]
        return [max_table_name]
    # Other modules have input tables forwarded to output
    else:
        return input_tables


def check_for_skippable_sort_operations(new_all_nodes, new_data_tables, node, hw_library):
    all_tables_sorted = True
    for table_name in new_all_nodes[node]["tables"]:
        if not is_table_sorted(new_data_tables[table_name], 0):
            all_tables_sorted = False
            break
    skipped_nodes = []
    # TODO: For now just remove the next sorting operation - Not entirely correct for the final product.
    if all_tables_sorted:
        for current_next_node_name in new_all_nodes[node]["after"]:
            if "sorting" in hw_library[new_all_nodes[current_next_node_name]["operation"]]["decorators"]:
                skipped_nodes.append(current_next_node_name)
    return skipped_nodes


def update_all_nodes(all_nodes, bitstream, data_tables, hw_library, node, node_cost, operation):
    bitstream_capacity = hw_library[operation]["bitstreams"][bitstream]["capacity"]
    missing_utility = []
    new_all_nodes = all_nodes.copy()
    new_data_tables = data_tables.copy()
    skipped_nodes = []
    if "sorting" in hw_library[operation]["decorators"]:
        # Update table
        should_node_be_removed = update_node_data_tables(all_nodes, node, bitstream_capacity,
                                                         hw_library[operation]["decorators"],
                                                         data_tables, new_data_tables)
        skipped_nodes = check_for_skippable_sort_operations(
            new_all_nodes, new_data_tables, node, hw_library)
    else:
        # Find missing utility
        should_node_be_removed = find_missing_utility(
            bitstream_capacity, missing_utility, node_cost)
        update_graph_capacities(
            all_nodes, missing_utility, new_all_nodes, node, should_node_be_removed)
    # Update all nodes
    if should_node_be_removed:
        resulting_tables = get_resulting_tables(all_nodes[node]["tables"],
                                                hw_library[operation]["decorators"],
                                                new_data_tables)
        update_next_node_tables(
            all_nodes, node, new_all_nodes, skipped_nodes, resulting_tables)
    return new_all_nodes, new_data_tables, should_node_be_removed, skipped_nodes


def update_next_node_tables(all_nodes, node, new_all_nodes, skipped_nodes, resulting_tables):
    add_new_table_to_next_nodes(
        all_nodes, new_all_nodes, node, resulting_tables)
    del new_all_nodes[node]
    # Assuming postorder tree structure of skipped_nodes
    # TODO: Check postorder
    for skipped_node in skipped_nodes:
        add_new_table_to_next_nodes(
            all_nodes, new_all_nodes, skipped_node, resulting_tables)
        del new_all_nodes[skipped_node]


def update_graph_capacities(all_nodes, missing_utility, new_all_nodes, node, should_node_be_removed):
    if not should_node_be_removed:
        new_all_nodes[node] = all_nodes[node].copy()
        new_capacity_values = []
        for capacity_parameter_index in range(len(missing_utility)):
            if missing_utility[capacity_parameter_index] <= 0:
                new_capacity_values.append(0)
            else:
                new_capacity_values.append(
                    missing_utility[capacity_parameter_index])
        new_all_nodes[node]["capacity"] = tuple(new_capacity_values)


def find_missing_utility(bitstream_capacity, missing_utility, node_cost):
    should_node_be_removed = True
    if len(bitstream_capacity) != len(node_cost):
        raise ValueError("Capacity parameters don't match")
    for capacity_parameter_index in range(len(bitstream_capacity)):
        missing_utility.append(
            node_cost[capacity_parameter_index] - bitstream_capacity[capacity_parameter_index])
        if bitstream_capacity[capacity_parameter_index] < node_cost[capacity_parameter_index]:
            should_node_be_removed = False
    return should_node_be_removed


def select_according_to_preferences(available_module_placements, module_placement_selections):
    all_selected_placements = []  # For ORing together at the end
    for module_placement_clause in module_placement_selections:
        current_selected_placements = available_module_placements.copy()  # For ANDing together
        for placement_selction_function in module_placement_clause:
            current_selected_placements = placement_selction_function(
                current_selected_placements)
        all_selected_placements.append(current_selected_placements)
    chosen_module_placements = []
    for chosen_module_set in all_selected_placements:
        for chosen_module in chosen_module_set:
            if chosen_module not in chosen_module_placements:
                chosen_module_placements.append(chosen_module)
    return chosen_module_placements


def get_scheduled_modules_for_node_after_pos(all_nodes, min_position, node, taken_columns, hw_library,
                                             module_placement_selections, skipped_placements):
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
                (chosen_module_position,
                 ScheduledModule(node, current_operation, chosen_bitstream, (chosen_column_position, end_index))))
        # For stats gathering: 0 - alternatives; 1 - nodes placed
        skipped_placements[0] += len(available_module_placements)
        available_module_placements = select_according_to_preferences(available_module_placements,
                                                                      module_placement_selections)
        skipped_placements[0] -= len(available_module_placements)
        skipped_placements[1] += 1
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
    print(f"Unique plan count:{len(plans)}")
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


def add_all_splitting_modules_nodes_to_list(first_nodes, graph):
    all_stream_dependencies = []
    splitting_streams = []
    for node_name in graph.keys():
        for previous_node in graph[node_name]["before"]:
            if previous_node not in all_stream_dependencies:
                all_stream_dependencies.append(previous_node)
            elif previous_node not in splitting_streams:
                splitting_streams.append(previous_node)
    for splitting_node in splitting_streams:
        for node in graph[splitting_node[0]]["after"]:
            if splitting_node in graph[node]["before"] and node not in first_nodes:
                first_nodes.append(node)


def add_all_first_modules_nodes_to_list(first_nodes, graph, hw_library):
    for node_name, node_parameters in graph.items():
        if "first_module" in hw_library[node_parameters["operation"]]["decorators"] and node_name not in first_nodes:
            first_nodes.append(node_name)


def get_first_nodes_from_saved_nodes(saved_nodes, graph):
    first_nodes = []
    for saved_node in saved_nodes:
        for first_node in graph[saved_node]["after"]:
            if first_node not in first_nodes:
                first_nodes.append(first_node)
    return first_nodes


def find_plans_and_print(starting_nodes, graph, resource_string, hw_library, data_tables, saved_nodes,
                         module_placement_selections):
    # print_node_placement_permutations(starting_nodes, graph)
    # print(f"{FancyText.UNDERLINE}Plans with placed modules below:{FancyText.END}")
    # Now with string match checks
    resulting_plans = []
    min_runs_pointer = [maxsize]
    start_time = perf_counter()
    skipped_placements_stat_pointer = [0, 0]
    first_nodes = get_first_nodes_from_saved_nodes(saved_nodes, graph)
    add_all_first_modules_nodes_to_list(first_nodes, graph, hw_library)
    add_all_splitting_modules_nodes_to_list(first_nodes, graph)
    place_nodes_recursively(starting_nodes, [], graph, [], [], resulting_plans, True,
                            hw_library, min_runs_pointer, data_tables, module_placement_selections,
                            skipped_placements_stat_pointer, first_nodes)
    stop_time = perf_counter()
    print(f"Number of full plans generated: {len(resulting_plans)}")
    print(f"Number of nodes placed to runs: {skipped_placements_stat_pointer[1]}")
    print(f"Number of module placements discarded: {skipped_placements_stat_pointer[0]}")
    print(
        f"Number of discarded placements per node: {skipped_placements_stat_pointer[0] / skipped_placements_stat_pointer[1]:.3f}")
    resulting_plans = list(dict.fromkeys(resulting_plans))
    print_statistics(resulting_plans)
    print_all_runs_using_less_runs_than(
        resulting_plans, min_runs_pointer[0] + 1, resource_string)
    print(f"Elapsed time of the scheduler: {stop_time - start_time:.3f}s")


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
            "decorators": ["sorting", "partial_sort"]
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
            "decorators": ["composable", "blocking_sort", "sorting", "first_module"]
        }, "Merge Join": {
            "bitstreams": {
                "join.bit": {"locations": (5, 15, 25), "length": 6, "capacity": (), "string": "BDMDMM",
                             "is_backwards": False}},
            "start_locations": [[], [], [], [], [], ['join.bit'], [], [], [], [], [], [], [], [], [], ['join.bit'], [],
                                [],
                                [], [], [], [], [], [], [], ['join.bit'], [], [], [], [], []],
            "decorators": ["sorted_input", "largest_input_is_output"]
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
        "B": {"operation": "Multiplier", "capacity": (), "before": (("A", 0),), "after": (), "tables": [""]},
        "C": {"operation": "Global Sum", "capacity": (), "before": (), "after": (), "tables": ["test_table2"]}
    }
    q19_graph = {
        "FirstFilter": {"operation": "Filter", "capacity": (4, 2), "before": (), "after": ("LinSort",),
                        "tables": ["test_table"]},
        "LinSort": {"operation": "Linear Sort", "capacity": (), "before": (("FirstFilter", 0),),
                    "after": ("MergeSort",), "tables": [""]},
        "MergeSort": {"operation": "Merge Sort", "capacity": (), "before": (("LinSort", 0),), "after": ("Join",),
                      "tables": [""]},
        "Join": {"operation": "Merge Join", "capacity": (), "before": (("MergeSort", 0),), "after": ("SecondFilter",),
                 "tables": ["", "test_table2"]},
        "SecondFilter": {"operation": "Filter", "capacity": (12, 4), "before": (("Join", 0),), "after": ("Add",),
                         "tables": [""]},
        "Add": {"operation": "Addition", "capacity": (), "before": (("SecondFilter", 0),), "after": ("Mul",),
                "tables": [""]},
        "Mul": {"operation": "Multiplier", "capacity": (), "before": (("Add", 0),), "after": ("Sum",), "tables": [""]},
        "Sum": {"operation": "Global Sum", "capacity": (), "before": (("Mul", 0),), "after": (), "tables": [""]}
    }
    capacity_test = {
        "FirstFilter": {"operation": "Filter", "capacity": (4, 2), "before": (), "after": ("LinSort",),
                        "tables": ["test_table2"]},
        "LinSort": {"operation": "Linear Sort", "capacity": (), "before": (("FirstFilter", 0),), "after": ("Sum",),
                    "tables": [""]},
        "Sum": {"operation": "Global Sum", "capacity": (), "before": (("LinSort", 0),), "after": (), "tables": [""]}
    }
    filter_test = {
        "LinSort": {"operation": "Linear Sort", "capacity": (), "before": (), "after": ("MergeSort",),
                    "tables": ["huge_table"]},
        "MergeSort": {"operation": "Merge Sort", "capacity": (), "before": (("LinSort", 0),), "after": (),
                      "tables": [""]}}
    # Sorted sequence = ((begin, length, column_i),...); empty = unsorted
    tables = {
        "test_table": {"record_count": 10000, "sorted_sequences": ()},
        "test_table2": {"record_count": 10000, "sorted_sequences": ((0, 10000, 0),)},
        "huge_table": {"record_count": 210000, "sorted_sequences": ((0, 10, 0), (10, 20, 0),)}
    }
    test_hw = {
        "thing": {
            "bitstreams": {
                "multiplier.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (), "string": "MBDMMBD",
                                   "is_backwards": False}},
            "start_locations": [['multiplier.bit'], [], [], [], [], [], [], [], [], [], ['multiplier.bit'], [], [], [],
                                [],
                                [], [], [], [], [], ['multiplier.bit'], [], [], [], [], [], [], [], [], [], []],
            "decorators": []
        }
    }
    test_graph = {
        "first": {"operation": "thing", "capacity": (), "before": (), "after": ("secondA", "secondB", "secondC"),
                  "tables": ["test_table", "test_table2"]},
        "secondA": {"operation": "thing", "capacity": (), "before": (("first", 0),), "after": (),
                    "tables": [""]},
        "secondB": {"operation": "thing", "capacity": (), "before": (("first", 0),), "after": (),
                    "tables": [""]},
        "secondC": {"operation": "thing", "capacity": (), "before": (("first", 1),), "after": (),
                    "tables": [""]}}

    # starting_nodes = ["A", "C"]
    # find_plans_and_print(starting_nodes, graph, resource_string, hw_library, tables, [], [[ModuleSelection.ALL_AVAILABLE]])
    # capacity_test_nodes = ["FirstFilter"]
    # find_plans_and_print(capacity_test_nodes, capacity_test,
    #                      resource_string, hw_library, tables, [], [[ModuleSelection.ALL_AVAILABLE]])
    # filter_test_nodes = ["LinSort"]
    # find_plans_and_print(filter_test_nodes, filter_test,
    #                      resource_string, hw_library, tables, [], [[ModuleSelection.FIRST_AVAILABLE, ModuleSelection.SHORTEST], [ModuleSelection.LAST_AVAILABLE, ModuleSelection.SHORTEST]])
    #
    q19_starting_nodes = ["FirstFilter"]
    find_plans_and_print(q19_starting_nodes, q19_graph,
                         resource_string, hw_library, tables, [], [[ModuleSelection.FIRST_AVAILABLE]])
    # find_plans_and_print(["first"], test_graph,
    #                      resource_string, test_hw, tables, [], [[ModuleSelection.ALL_AVAILABLE]])


if __name__ == '__main__':
    main()
    # Missing parts:
    # 2. Backwards paths
    # Decorators:
    #   Reducing
    # Optimisation rules:
    #   Reordering
    #   Data duplication for filters
    #   Actually linear sort removal is also an option
    # Fixing filter and sorting parameters
