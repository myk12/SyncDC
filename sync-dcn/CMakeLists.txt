cmake_minimum_required(VERSION 3.12)

# Set C++ standard requirements
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(TOPO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/topology")

# Machine Learning Project
project(SyncDCAllReduce)

include_directories(${ML_SOURCE_DIR} ${TOPO_SOURCE_DIR})
set(ML_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/machine-learning")

set(SOURCE_FILES
    ${ML_SOURCE_DIR}/ml-main.cc
    ${ML_SOURCE_DIR}/ml-common.cc
    ${ML_SOURCE_DIR}/app-all-reduce.cc
    ${ML_SOURCE_DIR}/app-ring-allreduce.cc
    ${ML_SOURCE_DIR}/app-ocs-allreduce.cc
    ${ML_SOURCE_DIR}/ocs-test.cc
    ${ML_SOURCE_DIR}/spine-leaf-test.cc
    ${TOPO_SOURCE_DIR}/spine-leaf.cc
    ${TOPO_SOURCE_DIR}/optical-circuit-switching.cc
)

set(HEADER_FILES
    ${ML_SOURCE_DIR}/ml-common.h
    ${ML_SOURCE_DIR}/app-all-reduce.h
    ${ML_SOURCE_DIR}/app-ring-allreduce.h
    ${ML_SOURCE_DIR}/app-ocs-allreduce.h
    ${TOPO_SOURCE_DIR}/spine-leaf.h
    ${TOPO_SOURCE_DIR}/optical-circuit-switching.h
)

add_executable(ml-allreduce ${SOURCE_FILES} ${HEADER_FILES})

# Link required NS-3 libraries
target_link_libraries(ml-allreduce
    ns3::network
    ns3::internet
    ns3::point-to-point
    ns3::csma
    ns3::applications
    ns3::flow-monitor
    # YAML C++ library
    yaml-cpp
)
