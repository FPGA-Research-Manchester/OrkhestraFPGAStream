/*
Copyright 2021 University of Manchester

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

#include "accelerator_library.hpp"

#include <stdexcept>

using orkhestrafs::dbmstodspi::AcceleratorLibrary;

void AcceleratorLibrary::SetupOperation(
    const AcceleratedQueryNode& node_parameters) {
  auto search = module_driver_library_.find(node_parameters.operation_type);
  if (search != module_driver_library_.end()) {
    recent_setup_modules_.clear();
    auto driver = search->second.get();
    // TODO: Put into a for loop for composed modules.
    auto current_module =
        driver->CreateModule(memory_manager_, node_parameters.operation_module_location);
    driver->SetupModule(*current_module, node_parameters);
    recent_setup_modules_.push_back(std::move(current_module));
  } else {
    throw std::runtime_error("Operator driver not found!");
  }
}

auto AcceleratorLibrary::GetDMAModule() -> std::unique_ptr<DMAInterface> {
  return dma_setup_->CreateDMAModule(memory_manager_);
}

auto AcceleratorLibrary::ExportLastModulesIfReadback()
    -> std::vector<std::unique_ptr<ReadBackModule>> {
  std::vector<std::unique_ptr<ReadBackModule>> resulting_vector;
  for (int i = 0; i < recent_setup_modules_.size(); i++) {
    auto cast_check =
        dynamic_cast<ReadBackModule*>(recent_setup_modules_.at(i).get());
    if (cast_check) {
      std::unique_ptr<ReadBackModule> my_new_pointer(
          dynamic_cast<ReadBackModule*>(
              std::move(recent_setup_modules_.at(i)).release()));
      resulting_vector.push_back(std::move(my_new_pointer));
    }
  }
  recent_setup_modules_.clear();
  return resulting_vector;
}

auto orkhestrafs::dbmstodspi::AcceleratorLibrary::GetDMAModuleSetup()
    -> DMASetupInterface& {
  return *dma_setup_;
}

auto orkhestrafs::dbmstodspi::AcceleratorLibrary::GetILAModule()
    -> std::unique_ptr<ILA> {
  // return std::make_unique<ILA>(memory_manager_.get());
  return nullptr;
}
