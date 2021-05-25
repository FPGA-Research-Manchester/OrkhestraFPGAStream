# EasyDSPI - Platform for using dynamic stream processing interfaces

This is just a prototype for now demonstrating how the structure of the platform will look like and then the DBMStoDSPI workflow is migrated to be an application of EasyDSPI.

## What is demonstrated?

Currently this prototype only demonstrates a few things not used in DBMStoDSPI.

### Core infrastructure

Part of the core where the execution and input parsing and possibly output data collection are separated with their own dependencies. These core parts of the code are completely independent thanks to the **core_interfaces** interface library. With factory methods giving the main core part of the program just a pointer defined by the interface it is easy and flexible to switch out different parts of the program as demonstrated by the input_manager. 

### JSON reading

Also a little bit of additional JSON reading is in this prototype project.

### State machine with bridge code design pattern

This is a very flexible way to define how the execution works and make it have fallback logic between states. The bridge code design pattern helps with different driver support.

Currently missing!

### Driver using decorators

This is a flexible way to add new driver functionalities to a generic driver without bloating a single generic driver.

Currently missing!

## What is missing?

This prototype doesn't include logic which is already present in DBMStoDSPI.

### JSON validation using schemas

Schema validation is possible with both [RapidJSON](https://rapidjson.org/md_doc_schema.html) and [JSON for Modern C++](https://github.com/nlohmann/json). Just the both the implementations and the actual schemas have to get developed which takes a bit of time.

### Multi threaded execution

Multi threaded execution would be important to try out on this prototype one day to see which parts of the program can be executed in parallel and how a CPU thread could simulate an accelerator running in the background.

### CSV reading

Done in DBMStoDSPI.

### Unit testing

Done in DBMStoDSPI and the whole structure is very unit testing friendly.

### FPGA usage

Done in DBMStoDSPI.

### Memory management

Done in DBMStoDSPI.