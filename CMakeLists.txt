project(kvbench-golang)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS -g)

include_directories(include)

# std::map
add_executable(std_map db/std_map.cc)
target_link_libraries(std_map pthread)

# combo tree
add_executable(combo_tree db/combo_tree.cc)
target_link_directories(combo_tree PUBLIC /home/qyzhang/Projects/ComboTree/build)
target_include_directories(combo_tree PUBLIC /home/qyzhang/Projects/ComboTree/include)
target_link_libraries(combo_tree combotree pthread)