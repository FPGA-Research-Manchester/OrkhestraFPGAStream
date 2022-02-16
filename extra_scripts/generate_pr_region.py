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
from collections import defaultdict
import random


# Give in percentages and give a bit to other on each placement for certain number of columns
def main(argv):
    example_string = "MMDMDBMMDBMMDMDBMMDBMMDMDBMMDBM"
    if not example_string:
        requirements = {"D": 0.3, "B": 0.1, "M": 0.6}
        length = 31
    length, requirements = get_requirements(example_string)

    generated_string = ""
    for i in range(length):
        chosen_resource = random.choices(list(requirements.keys()), weights=list(requirements.values()), k=1)[0]
        generated_string += chosen_resource
        for char in requirements.keys():
            if char == chosen_resource:
                requirements[char] -= 0.02
            else:
                requirements[char] += 0.01

    print(generated_string)


def get_requirements(example_string):
    counts = defaultdict(int)
    for char in example_string:
        counts[char] += 1
    length = len(example_string)
    requirements = {}
    for char in counts.keys():
        requirements[char] = float(counts[char]) / length
    return length, requirements


if __name__ == '__main__':
    main(sys.argv[1:])
