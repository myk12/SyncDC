cmake_minimum_required(VERSION 3.12)

# Set C++ standard requirements
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source files
set(SOURCE_FILES
    paxos-main.cc
    paxos-common.cc
    paxos-frame.cc
    paxos-app.cc
    paxos-app-client.cc
    paxos-app-server-listener.cc
    paxos-app-server-proposer.cc
    paxos-topology-clos.cc
)

# Add Headers
set(HEADER_FILES
    paxos-app.h
    paxos-frame.h
    paxos-common.h
    paxos-app-client.h
    paxos-topology-clos.h
)

# Specify executable
add_executable(sync-paxos ${SOURCE_FILES} ${HEADER_FILES})

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