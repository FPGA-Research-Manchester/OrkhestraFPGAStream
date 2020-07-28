#!/bin/bash

projectFiles=()

for entry in ./apps/*
do
  projectFiles+=($entry)
done

# Starts tidying with gmock. Better do sepparately.
#for entry in ./tests/*
#do
#  projectFiles+=($entry)
#done

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

#Below doesn't work nicely!
: "
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-avoid-bind' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-concat-nested-namespaces' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-deprecated-headers' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-deprecated-ios-base-aliases' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-loop-convert' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-make-shared' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-make-unique' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-pass-by-value' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-raw-string-literal' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-redundant-void-arg' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-replace-auto-ptr' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-replace-disallow-copy-and-assign-macro' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-replace-random-shuffle' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-return-braced-init-list' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-shrink-to-fit' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-unary-static-assert' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-auto' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-bool-literals' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-default-member-init' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-emplace' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-equals-default' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-equals-delete' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-nodiscard' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-noexcept' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-nullptr' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-override' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-trailing-return-type' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-transparent-functors' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-uncaught-exceptions' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,modernize-use-using' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug

clang-tidy --checks='-*,cppcoreguidelines-init-variables' --fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
clang-tidy --checks='-*,cppcoreguidelines-pro-bounds-constant-array-index' --fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
clang-tidy --checks='-*,cppcoreguidelines-pro-type-cstyle-cast' --fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
clang-tidy --checks='-*,cppcoreguidelines-pro-type-member-init' --fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
clang-tidy --checks='-*,cppcoreguidelines-pro-type-static-cast-downcast' --fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug

python ./run-clang-tidy.py -header-filter='.*' -checks='-*,google-readability-braces-around-statements' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,google-readability-function-size' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,google-readability-namespace-comments' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,google-explicit-constructor' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,google-upgrade-googletest-case' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug

python ./run-clang-tidy.py -header-filter='.*' -checks='-*,misc-definitions-in-headers' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,misc-redundant-expression' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,misc-static-assert' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,misc-uniqueptr-reset-release' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,misc-unused-alias-decls' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,misc-unused-parameters' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,misc-unused-using-decls' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug

python ./run-clang-tidy.py -header-filter='.*' -checks='-*,performance-faster-string-find' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,performance-for-range-copy' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,performance-inefficient-algorithm' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,performance-inefficient-vector-operation' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,performance-move-const-arg' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,performance-move-constructor-init' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,performance-noexcept-move-constructor' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,performance-trivially-destructible' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,performance-type-promotion-in-math-fn' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,performance-unnecessary-value-param' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug

python ./run-clang-tidy.py -header-filter='.*' -checks='-*,portability-restrict-system-includes' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug

python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-braces-around-statements' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-const-return-type' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-container-size-empty' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-delete-null-pointer' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-else-after-return' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-identifier-naming' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-implicit-bool-conversion' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-inconsistent-declaration-parameter-name' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-isolate-declaration' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-make-member-function-const' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-misplaced-array-index' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-named-parameter' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-non-const-parameter' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-qualified-auto' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-redundant-access-specifiers' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-redundant-control-flow' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-redundant-declaration' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-redundant-function-ptr-dereference' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-redundant-member-init' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-redundant-smartptr-get' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-redundant-string-cstr' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-redundant-string-init' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-simplify-boolean-expr' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-simplify-subscript-expr' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-static-accessed-through-instance' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-static-definition-in-anonymous-namespace' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-string-compare' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-uniqueptr-delete-release' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
python ./run-clang-tidy.py -header-filter='.*' -checks='-*,readability-uppercase-literal-suffix' -fix -p=C://Users//Kaspar//source//repos//DBMStoDSPI//out//build//x64-Debug
"
#clang-format -i -style=google **/*.cpp **/*.hpp
