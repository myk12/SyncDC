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
    ${ML_SOURCE_DIR}/ml-common.cc
    ${ML_SOURCE_DIR}/app-all-reduce.cc
    ${ML_SOURCE_DIR}/app-ring-allreduce.cc
    ${ML_SOURCE_DIR}/app-ocs-allreduce.cc
    ${ML_SOURCE_DIR}/app-fastpass-allreduce.cc
    ${ML_SOURCE_DIR}/app-fastpass-arbiter.cc
    ${ML_SOURCE_DIR}/header-fastpass.cc
    ${TOPO_SOURCE_DIR}/spine-leaf.cc
    ${TOPO_SOURCE_DIR}/optical-circuit-switching.cc
)

set(HEADER_FILES
    ${ML_SOURCE_DIR}/ml-common.h
    ${ML_SOURCE_DIR}/app-all-reduce.h
    ${ML_SOURCE_DIR}/app-ring-allreduce.h
    ${ML_SOURCE_DIR}/app-ocs-allreduce.h
    ${ML_SOURCE_DIR}/app-fastpass-allreduce.h
    ${ML_SOURCE_DIR}/app-fastpass-arbiter.h
    ${ML_SOURCE_DIR}/header-fastpass.h
    ${TOPO_SOURCE_DIR}/spine-leaf.h
    ${TOPO_SOURCE_DIR}/optical-circuit-switching.h
)

# Add executable target Ring-Allreduce Test
set(RING_ALLREDUCE_SOURCE
    tests/test-spine-leaf-ml-workload.cc
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

include_directories(${ML_SOURCE_DIR} ${TOPO_SOURCE_DIR})

add_executable(ml-ring-allreduce ${RING_ALLREDUCE_SOURCE}   )

# Link required NS-3 libraries
target_link_libraries(ml-ring-allreduce
    ns3::network
    ns3::internet
    ns3::point-to-point
    ns3::csma
    ns3::applications
    ns3::flow-monitor
    # YAML C++ library
    yaml-cpp
)


set(FAST_ALLREDUCE_SOURCE
    tests/test-fastpass-ml-workload.cc
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

add_executable(ml-fast-allreduce ${FAST_ALLREDUCE_SOURCE}   )

# Link required NS-3 libraries
target_link_libraries(ml-fast-allreduce
    ns3::network
    ns3::internet
    ns3::point-to-point
    ns3::csma
    ns3::applications
    ns3::flow-monitor
    # YAML C++ library
    yaml-cpp
)
