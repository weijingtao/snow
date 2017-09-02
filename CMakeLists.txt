cmake_minimum_required(VERSION 3.3)
project(snow CXX)

set(CMAKE_CXX_FLAGS "-std=c++14 -DBOOST_COROUTINES_NO_DEPRECATION_WARNING")

file(GLOB SNOW_SOURCE_FILES ./*.hpp)
aux_source_directory(. SOURCE_FILES)


add_executable(test1 ${SOURCE_FILES} ${SNOW_SOURCE_FILES})
target_link_libraries(test1 boost_log_setup boost_log boost_context boost_coroutine boost_system yaml-cpp pthread)