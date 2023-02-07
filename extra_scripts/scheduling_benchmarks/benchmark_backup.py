# Copyright 2023 University of Manchester
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


import json
import sys
import gzip


# Write last entry.
def backup(graph_filename, table_filename, backup_filename):
    result_data = {"graph":dict(), "table":list()}
    with open(graph_filename, "r") as graph_file:
        result_data["graph"] = json.loads(graph_file.read())
    with open(table_filename, "r") as table_file:
        result_data["table"] = json.loads(table_file.read())
    with gzip.open(backup_filename, 'ab') as f_out:
        f_out.write(json.dumps(result_data).encode() + '\n'.encode())


def restore(graph_filename, table_filename, backup_filename, query_id):
    backup_data = dict()
    with gzip.open(backup_filename, 'rb') as f:
        for i, line in enumerate(f):
            if i == query_id:
                backup_data = json.loads(line.decode())
    with open(graph_filename, "w") as f:
        json.dump(backup_data["graph"], f)
    with open(table_filename, "w") as f:
        json.dump(backup_data["table"], f)


def main(argv):
    pass
    # Need 4 arguments: python3 benchmark_backup.py $graph_filename $table_filename $backup_file backup/restore(-1/N)
    is_backup_mode = int(argv[3]) == -1

    graph_file = argv[0]
    table_file = argv[1]
    backup_file = argv[2]

    if is_backup_mode:
        backup(graph_file, table_file, backup_file)
    else:
        restore(graph_file, table_file, backup_file, int(argv[3]))


if __name__ == '__main__':
    main(sys.argv[1:])
