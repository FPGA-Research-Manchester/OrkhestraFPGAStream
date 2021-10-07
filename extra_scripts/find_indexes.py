# Press Shift+F10 to execute it

# https://stackoverflow.com/questions/4664850/how-to-find-all-occurrences-of-a-substring
def find_all_start_indexes(base_string, substring):
    start = 0
    while True:
        start = base_string.find(substring, start)
        if start == -1:
            return
        yield start
        start += 1


def find_all_start_indexes_for_a_module(base_string, substrings):
    print(substrings)
    all_indexes = []
    start_locations = [[] for _ in range(len(base_string))]
    for sub in substrings:
        start_indexes = list(find_all_start_indexes(base_string, sub))
        all_indexes.append(start_indexes)
        # print(f"{sub}:{start_indexes}")
        for index in start_indexes:
            start_locations[index].append(sub)
    print(all_indexes)
    print(start_locations)


if __name__ == '__main__':
    # base = input("Base string: ")
    # sub = input("Substring: ")
    # start_indexes = list(find_all_start_indexes(base, sub))

    base = "MBDMMBDMDMMBDMMBDMDMMBDMMBDMDMM"
    filter_substrings = ["MDMM", "DMMBDMD", "DMDMM", "MBDMDMM", "DMDMM", "DMMBDMD", "BDMMBDMDMM"]
    linear_substrings = ["BDMMBDMDMM", "BDMMBDMDMMBDMM"]
    merge_sort_substrings = ["MBDMDMM", "BDMMBDMDMM", "BDMMBDMDMMBD"]
    join_substrings = ["BDMDMM"]
    addition_substrings = ["MDMM"]
    multiplier_substrings = ["MBDMMBD"]
    global_sum_substrings = ["DMD"]
    print("Filter:")
    find_all_start_indexes_for_a_module(base, filter_substrings)
    print("Linear:")
    find_all_start_indexes_for_a_module(base, linear_substrings)
    print("Merge Sort:")
    find_all_start_indexes_for_a_module(base, merge_sort_substrings)
    print("Join:")
    find_all_start_indexes_for_a_module(base, join_substrings)
    print("Addition:")
    find_all_start_indexes_for_a_module(base, addition_substrings)
    print("Multiplier:")
    find_all_start_indexes_for_a_module(base, multiplier_substrings)
    print("Global Sum:")
    find_all_start_indexes_for_a_module(base, global_sum_substrings)
