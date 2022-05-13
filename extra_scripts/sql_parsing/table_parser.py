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


def main(argv):
    argv.append("lineitem_table.txt")
    with open(argv[0]) as f:
        lines = f.readlines()[3:]

    for line in lines:
        values = line.split("|")[:2]
        for value in values:
            print(value.strip())


if __name__ == '__main__':
    main(sys.argv[1:])
