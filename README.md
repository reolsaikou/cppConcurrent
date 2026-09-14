### CMake

#### Chapter 4

- powerful features for scheduling dependencies between targets and how libraries are linked are provided by CMake
- some basic uses of parameters and properties are also given to add a flavor for how these features function

##### 4.2

- CMake supports native distinct features of each platforms.
- Some supported but deprecated uses
- name Targets not related to project name.
- name lib's Targets without a preceding lib.
- avoid specifying the STATIC or SHARED keyword
- the three keywords have a strong impact on how inter-targets dependencies are managed.
