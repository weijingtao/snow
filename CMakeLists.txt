cmake_minimum_required(VERSION 3.3)
project(snow CXX)

set(CMAKE_CXX_FLAGS "-std=c++14 -DBOOST_COROUTINES_NO_DEPRECATION_WARNING")

file(GLOB SNOW_SOURCE_FILES ./*.hpp client/*.hpp utils/*.hpp client/detail/*.hpp)

include_directories(.)
aux_source_directory(. SOURCE_FILES)


add_executable(test1 test1.cpp ${SNOW_SOURCE_FILES})
target_link_libraries(test1 boost_log_setup boost_log boost_context boost_coroutine boost_system yaml-cpp pthread)

add_executable(test2 test2.cpp ${SNOW_SOURCE_FILES})
target_link_libraries(test2
        boost_log_setup
        boost_log
        boost_context
        boost_coroutine
        boost_system
        yaml-cpp
        pthread)