#!/bin/bash
# Script to run clang-tidy automated checks and fixes. To omit a line use //NOLINT

projectFiles=()

for entry in ./apps/*
do
  projectFiles+=($entry)
done

# Doesn't work well with gtest and gmock. Try doing sepparately.
for entry in ./tests/*
do
  projectFiles+=($entry)
done

for entry in ./src/*
do
  projectFiles+=($entry)
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
    for check in ${checks[@]}
    do
        echo $file $check
        clang-tidy --checks="-*,$check" --fix-errors --quiet -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug $file | grep "clang-tidy applied"
    done
done

clang-format -i -style=google **/*.cpp **/*.hpp
