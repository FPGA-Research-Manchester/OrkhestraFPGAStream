﻿/*
Copyright 2022 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "sql_parser.hpp"

#include <iostream>
#include <set>
#include <unordered_set>

#include "sql_json_reader.hpp"

using orkhestrafs::core_interfaces::table_data::kDataTypeNames;
using orkhestrafs::sql_parsing::SQLJSONReader;
using orkhestrafs::sql_parsing::SQLParser;

void SQLParser::CreatePlan(SQLQueryCreator& sql_creator,
                           std::string query_filename) {
  const std::string default_database_name = "tpch_001";
  std::cout << "Parsing: " << query_filename << std::endl;
  std::map<int, std::vector<std::string>> explain_data;
  // TODO: Should do more checks!
  if (std::system(nullptr)) {
    std::string command = "python3 postgresql_explain_parser.py";
    command += " " + default_database_name + " " + query_filename;
    std::cout << command << std::endl;
    std::system(command.c_str());
  } else {
    throw std::runtime_error("Can't execute any subprocesses");
  }
  // TODO: Hardcoded for now
  std::string parsed_filename = "parsed.json";
  // Actually the query needs to get parsed by the Python script first!
  SQLJSONReader::ReadQuery(parsed_filename, explain_data);
  // After reading the data in you just use the API to write everything in.
  //
  // You have a bunch of params you don't know what to do with them
  // Parse them into an actual tree?
  // Find all Tables
  // Put them into another map with tables and columns and column data
  // Then Put COMP stuff into another map
  // And the rest put into a third map
  //
  // While you are putting things into a map you also attach the inputs. You
  // create a map which is int -> set{int} All places where it is needed I also
  // need the reverse map -> To reduce it until it is 0 to say I can now process
  // it.
  //
  // All numbers have also the number from sql creator attached to them.
  //
  // Then You parse the table - Check all thing in the rest that have the table
  // as an input

  // Initial data parsing to get dependencies.
  // For key to be available all input_dependencies must be done!
  std::map<int, std::set<int>> input_dependencies;
  // Once you get key done you can check if one of the outputs can be done next!
  std::map<int, std::set<int>> output_dependencies;
  std::map<int, std::string> registered_entities_map;
  std::map<int, int> registered_comparisons;
  std::map<std::string, ColumnDataType> column_types;
  std::set<int> available_to_register;
  const std::unordered_set<std::string> default_operations = {
      "Aggregate", "Filter", "Multiplication", "Addition"};
  const std::unordered_map<std::string, CompareFunctions> comparison_functions =
      {
          {"<", CompareFunctions::kLessThan},
          {"<=", CompareFunctions::kGreaterThanOrEqual},
          {"=", CompareFunctions::kEqual},
          {"!=", CompareFunctions::kNotEqual},
          {">", CompareFunctions::kGreaterThan},
          {">=", CompareFunctions::kGreaterThanOrEqual}
      };
  // Should be made more "pretty"
  // Assuming params are correct!
  for (const auto& [key, params] : explain_data) {
    std::set<int> key_set = {key};
    if (params[0] == "Column") {
      column_types.insert({params[1], kDataTypeNames.at(params[3])});
    } else if (params[0] == "Columns") {
      // Do nothing
    } else if (params[0] == "Table") {
      available_to_register.insert(key);
    } else if (params[0] == "Join") {
      input_dependencies.insert(
          {key, {std::stoi(params[1]), std::stoi(params[3])}});
      if (const auto& [it, inserted] =
              output_dependencies.try_emplace(std::stoi(params[1]), key_set);
          !inserted) {
        it->second.insert(key);
      }
      if (const auto& [it, inserted] =
              output_dependencies.try_emplace(std::stoi(params[3]), key_set);
          !inserted) {
        it->second.insert(key);
      }
    } else if (default_operations.find(params[0]) != default_operations.end()) {
      input_dependencies.insert({key, {std::stoi(params[1])}});
      if (const auto& [it, inserted] =
              output_dependencies.try_emplace(std::stoi(params[1]), key_set);
          !inserted) {
        it->second.insert(key);
      }
    } else if (params[0] == "COMP") {
      input_dependencies.insert({key, {std::stoi(params[1])}});
      if (const auto& [it, inserted] =
              output_dependencies.try_emplace(std::stoi(params[1]), key_set);
          !inserted) {
        it->second.insert(key);
      }
      if (params[2] == "NOT") {
        input_dependencies[key].insert(std::stoi(params[3]));
        if (const auto& [it, inserted] =
                output_dependencies.try_emplace(std::stoi(params[3]), key_set);
            !inserted) {
          it->second.insert(key);
        }
      } else if (params[2] == "OR" || params[2] == "AND") {
        input_dependencies[key].insert(std::stoi(params[3]));
        if (const auto& [it, inserted] =
                output_dependencies.try_emplace(std::stoi(params[3]), key_set);
            !inserted) {
          it->second.insert(key);
        }
        input_dependencies[key].insert(std::stoi(params[4]));
        if (const auto& [it, inserted] =
                output_dependencies.try_emplace(std::stoi(params[4]), key_set);
            !inserted) {
          it->second.insert(key);
        }
      } else {
        // Just a comparison (hopefully) - do nothing
      }
    } else {
      throw std::runtime_error("Unknown node type!");
    }
  }

  while (!available_to_register.empty()) {
    auto current_op_node = *available_to_register.begin();
    available_to_register.erase(available_to_register.begin());
    if (output_dependencies.find(current_op_node) !=
        output_dependencies.end()) {
      for (const auto output_key : output_dependencies.at(current_op_node)) {
        input_dependencies[output_key].erase(current_op_node);
        if (input_dependencies[output_key].empty()) {
          available_to_register.insert(output_key);
        }
      }
    }

    if (explain_data.at(current_op_node).front() == "COMP") {
      if (explain_data.at(current_op_node).at(2) == "AND") {
        registered_comparisons.insert(
            {current_op_node,
             sql_creator.AddAnd(
                 registered_entities_map.at(
                     std::stoi(explain_data.at(current_op_node).at(1))),
                 {registered_comparisons.at(
                      std::stoi(explain_data.at(current_op_node).at(3))),
                  registered_comparisons.at(
                      std::stoi(explain_data.at(current_op_node).at(4)))})});
      } else if (explain_data.at(current_op_node).at(2) == "OR") {
        registered_comparisons.insert(
            {current_op_node,
             sql_creator.AddOr(
                 registered_entities_map.at(
                     std::stoi(explain_data.at(current_op_node).at(1))),
                 {registered_comparisons.at(
                      std::stoi(explain_data.at(current_op_node).at(3))),
                  registered_comparisons.at(
                      std::stoi(explain_data.at(current_op_node).at(4)))})});
      } else if (explain_data.at(current_op_node).at(2) == "NOT") {
        registered_comparisons.insert(
            {current_op_node,
             sql_creator.AddNot(
                 registered_entities_map.at(
                     std::stoi(explain_data.at(current_op_node).at(1))),
                 {registered_comparisons.at(
                     std::stoi(explain_data.at(current_op_node).at(3)))})});
      } else {
        auto compare_function =
            comparison_functions.at(explain_data.at(current_op_node).at(2));
        // TODO: Check that the column is the first or second argument!
        auto datatype = column_types.at(explain_data.at(current_op_node).at(3));
        switch (datatype) {
          case ColumnDataType::kInteger:
            registered_comparisons.insert(
                {current_op_node,
                 sql_creator.AddIntegerComparison(
                     registered_entities_map.at(
                         std::stoi(explain_data.at(current_op_node).at(1))),
                     explain_data.at(current_op_node).at(3), compare_function,
                      std::stoi(explain_data.at(current_op_node).at(4)))});
            break;
          case ColumnDataType::kVarchar:
            registered_comparisons.insert(
                {current_op_node,
                 sql_creator.AddStringComparison(
                     registered_entities_map.at(
                         std::stoi(explain_data.at(current_op_node).at(1))),
                     explain_data.at(current_op_node).at(3), compare_function,
                     explain_data.at(current_op_node).at(4))});
            break;
          case ColumnDataType::kDecimal:
            registered_comparisons.insert(
                {current_op_node,
                 sql_creator.AddDoubleComparison(
                     registered_entities_map.at(
                         std::stoi(explain_data.at(current_op_node).at(1))),
                     explain_data.at(current_op_node).at(3), compare_function,
                     std::stod(explain_data.at(current_op_node).at(4)))});
            break;
          case ColumnDataType::kDate:
            throw std::runtime_error("Unsupported data type still!");
          default:
            throw std::runtime_error("Unsupported data type!");
        }
      }
    } else if (explain_data.at(current_op_node).front() == "Table") {
      std::vector<TableColumn> columns;
      for (const auto& column_key :
           explain_data.at(std::stoi(explain_data.at(current_op_node).at(2)))) {
        if (column_key == "Columns") {
          // Ignore
        } else {
          auto& column_params = explain_data.at(std::stoi(column_key));
          columns.emplace_back(kDataTypeNames.at(column_params.at(3)),
                               std::stoi(column_params.at(2)),
                               column_params.at(1));
        }
      }
      registered_entities_map.insert(
          {current_op_node,
           sql_creator.RegisterTable(
               explain_data.at(current_op_node).at(3), columns,
               std::stoi(explain_data.at(current_op_node).at(1)))});
    } else if (explain_data.at(current_op_node).front() == "Join") {
      registered_entities_map.insert(
          {current_op_node, sql_creator.RegisterJoin(
                                registered_entities_map.at(std::stoi(
                                    explain_data.at(current_op_node).at(1))),
                                explain_data.at(current_op_node).at(2),
                                registered_entities_map.at(std::stoi(
                                    explain_data.at(current_op_node).at(3))),
                                explain_data.at(current_op_node).at(4))});
    } else if (explain_data.at(current_op_node).front() == "Aggregate") {
      registered_entities_map.insert(
          {current_op_node, sql_creator.RegisterAggregation(
                                registered_entities_map.at(std::stoi(
                                    explain_data.at(current_op_node).at(1))),
                                explain_data.at(current_op_node).at(2))});
    } else if (explain_data.at(current_op_node).front() == "Filter") {
      registered_entities_map.insert(
          {current_op_node,
           sql_creator.RegisterFilter(registered_entities_map.at(
               std::stoi(explain_data.at(current_op_node).at(1))))});
    } else if (explain_data.at(current_op_node).front() == "Multiplication") {
      registered_entities_map.insert(
          {current_op_node, sql_creator.RegisterMultiplication(
                                registered_entities_map.at(std::stoi(
                                    explain_data.at(current_op_node).at(1))),
                                explain_data.at(current_op_node).at(2),
                                explain_data.at(current_op_node).at(3),
                                explain_data.at(current_op_node).at(4))});
    } else if (explain_data.at(current_op_node).front() == "Addition") {
      registered_entities_map.insert(
          {current_op_node,
           sql_creator.RegisterAddition(
               registered_entities_map.at(
                   std::stoi(explain_data.at(current_op_node).at(1))),
               explain_data.at(current_op_node).at(2),
               explain_data.at(current_op_node).at(3) == "TRUE",
               std::stoi(explain_data.at(current_op_node).at(4)))});
    } else {
      throw std::runtime_error("Unknown node type unexpectedly!");
    }
  }
}