cmake_minimum_required(VERSION 3.12)
project(SyncDC)

# Set C++ standard requirements
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# NS-3 configuration
set(NS3_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ns-3.45")

# Add NS-3 subdirectory
add_subdirectory(${NS3_SOURCE_DIR} ${CMAKE_BINARY_DIR}/ns-3_build)

# Link required NS-3 libraries
link_libraries(ns3::network ns3::internet ns3::point-to-point ns3::applications)

# Paxos module
set(PAXOS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sync-paxos")
add_subdirectory(${PAXOS_SOURCE_DIR} ${CMAKE_BINARY_DIR}/sync-paxos_build)