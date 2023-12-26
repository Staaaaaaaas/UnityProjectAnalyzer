## About
This is a test task for a JetBrains internship project.
A console tool that analyzes Unity project directory and dumps following information about the project:
* Scene hierarchy
* Unused scripts `(.cs)`

## Dependencies
The parsing of the YAML files is done with help of [yaml-cpp](https://github.com/jbeder/yaml-cpp).
You should first set up this library under `/yaml-cpp` for this tool to work, by running:
`git submodule add https://github.com/jbeder/yaml-cpp.git`.
## Usage
1. Build this project with CMake.
2. The built executable file will be saved to `cmake-build-debug` by the name `tool`.
3. Run the executable with `./tool.exe unity_project_path output_folder_path`

