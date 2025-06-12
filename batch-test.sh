#!/bin/bash

# First compile the program
mkdir -p build && cd build && cmake .. && make
cd ../

# Check if executable build/bin/sync-paxos exists
if [ ! -f build/bin/sync-paxos ]; then
    echo "Error: build/bin/sync-paxos does not exist"
    exit 1
fi

# Change parameters and run the program

# Sync Error Parameter Array
sync_error_arr=(100000 10000 1000 100 10)
# Message Delay Bound Array
delay_bound_arr=(100000000 10000000 1000000 100000 10000)

if [ -d result ]; then
    rm -rf result
    mkdir result
fi

for i in "${sync_error_arr[@]}"
do
    for j in "${delay_bound_arr[@]}"
    do
        mkdir -p result/SyncErr_$i/MsgDelay_$j

        ./build/bin/sync-paxos --clockSyncError=${i}ns --boundedMessageDelay=${j}ns

        # move the result to the result folder
        mv *.dat result/SyncErr_$i/MsgDelay_$j/
    done
done