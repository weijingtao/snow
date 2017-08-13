cmake_minimum_required(VERSION 3.3)
project(snow)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -DBOOST_COROUTINES_NO_DEPRECATION_WARNING")

set(SOURCE_FILES)
add_executable(test1 test1.cpp)
target_link_libraries(test1 boost_log_setup boost_log boost_context boost_coroutine boost_system pthread)