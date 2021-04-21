# DBMStoDSPI implementation details

Different concepts on how to use and how does the software work with the hardware designes is described in the documents below:

- [Crossbar configuration data](./crossbar_configuration.md)
  - In-detph description of the crossbar configuration between the hardware stream processing interface and AXI ports.
- [Input CSV file configuration](./input_data_configuration.md)
  - Brief explanation of how the database data is expected to be passed to this program.
- [Acceleration module characteristics](./acceleration_modules.md)
  - Document classifying currently supported acceleration modules and what kind of characteristics have to be taken into consideration to support such modules.
- [Program input](./program_input.md)
  - Specification document on how to use this software which links to all of the previous documenation pages.
- [Memory usage](./memory_allocation.md)
  - Explanation of how memory is allocated using FOS. And how usable memory is pinned with Linux.
- [End vision](./vision.md)
  - Brief explanation how this software could be extended to become a fully configurable engine not only supporting the current DBMS workflow.