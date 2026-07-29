cmake_minimum_required(VERSION 3.16)
project(sensor_node_host LANGUAGES CXX)

# Plain host build of the hardware-agnostic logic in src/, plus its
# GoogleTest-based unit tests in tests/. This is NOT the firmware build —
# see firmware/ for the ESP-IDF project targeting the esp-wroom-32e.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
