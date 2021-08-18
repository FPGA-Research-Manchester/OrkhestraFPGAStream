#!/bin/bash
# Script to run clang-tidy automated checks and fixes. To omit a line use //NOLINT

projectFiles=()

for entry in ./apps/main.cpp
do
  projectFiles+=($entry)
done

for entry in ./src/fpga_managing/*
do
  if [ ! -d "$entry" ]; then
      projectFiles+=($entry)
  fi
done

for entry in ./src/fpga_managing/modules/*
do
  projectFiles+=($entry)
done

for entry in ./src/data_managing/*
do
  projectFiles+=($entry)
done

for entry in ./src/query_managing/*
do
  projectFiles+=($entry)
done

for entry in ./src/input_managing/*
do
  projectFiles+=($entry)
done

# If it doesn't work well with gtest and gmock try doing these files sepparately.
for entry in ./tests/*
do
  if [ ! -d "$entry" ]; then 
    projectFiles+=($entry)
  fi
done

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
