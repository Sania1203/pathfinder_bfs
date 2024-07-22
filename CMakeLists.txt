cmake_minimum_required(VERSION 3.5)
project(pathfinder)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find_package(<dependency> REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(in_map src/node1_in_map.cpp)
ament_target_dependencies(in_map rclcpp std_msgs)

add_executable(algo_out src/node2_algo_out.cpp)
ament_target_dependencies(algo_out rclcpp std_msgs)

install(TARGETS
  in_map
  algo_out
  DESTINATION lib/${PROJECT_NAME})

ament_package()
