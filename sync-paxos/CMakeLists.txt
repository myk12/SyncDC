cmake_minimum_required(VERSION 3.12)

# Set C++ standard requirements
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add executable
add_executable(sync-paxos paxos-main.cc)

# Link required NS-3 libraries
target_link_libraries(sync-paxos
    ns3::network
    ns3::internet
    ns3::point-to-point
    ns3::csma
    ns3::applications
)

# Include directories
target_include_directories(sync-paxos PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${NS3_INCLUDE_DIRS}
)

# Compiler definitions
target_compile_definitions(sync-paxos PRIVATE
    NS3_LOG_ENABLE
)