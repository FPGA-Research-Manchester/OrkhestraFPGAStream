#!/bin/bash
# Script to run clang-tidy automated checks and fixes. To omit a line use //NOLINT

projectFiles=()

set_project_files () {
  for entry in $1
  do
    if [ ! -d "$entry" ]; then
      projectFiles+=($entry)
    fi
  done
}

set_project_files "./apps/main.cpp"
set_project_files "./src/core/*"
set_project_files "./src/core/core_execution/*"
set_project_files "./src/core/core_input/*"
set_project_files "./src/core_interfaces/*"
set_project_files "./src/dbmstodspi/input_handling/*"
set_project_files "./src/dbmstodspi/query_execution/*"
set_project_files "./src/dbmstodspi/query_execution/fpga_managing/*"
set_project_files "./src/dbmstodspi/query_execution/fpga_managing/modules/*"
set_project_files "./src/dbmstodspi/query_execution/fpga_managing/setup/*"
set_project_files "./src/dbmstodspi/query_execution/scheduling/*"
set_project_files "./src/dbmstodspi/query_execution/states/*"
set_project_files "./src/dbmstodspi/query_execution/table_data/*"
set_project_files "./src/dbmstodspi/util/*"

tempArray=()
for file in ${projectFiles[@]}
do
  if [[ "$file" != *".txt" ]]
    then
        tempArray+=($file)
  fi
done
projectFiles=("${tempArray[@]}")
unset tempArray

checks=(
modernize-*
google-*
cppcoreguidelines-*
bugprone-*
readability-*
misc-*
performance-*
portability-*
)

for file in ${projectFiles[@]}
do
    echo $file
    for check in ${checks[@]}
    do
        echo $check
        clang-tidy --checks="-*,$check" --fix-errors --quiet -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//Local\ PC $file
        echo
    done
    echo
done
echo "Automatic fixes done!"
clang-format -i -style=google **/*.cpp **/*.hpp
echo "Formatting done!"
