#!/bin/bash

# First compile the program
mkdir -p build && cd build && cmake .. && make
cd ../

# Check if executable build/bin/sync-paxos exists
if [ ! -f build/bin/sync-paxos ]; then
    echo "Error: build/bin/sync-paxos does not exist"
    exit 1
fi

if [ -d result ]; then
    rm -rf result
    mkdir result
fi

################################################
#        Synchronous Paxos
################################################    
# Change parameters and run the program
# Message Delay Bound Array
delay_bound_arr=(50000 5000 500 50 5)
# Sync Error Parameter Array
sync_error_arr=(50000 5000 500 50 5)

mkdir -p result/Sync

for i in "${delay_bound_arr[@]}"
do
    for j in "${sync_error_arr[@]}"
    do
        mkdir -p result/Sync/Delay_${i}us/Sync_${j}ns
       
        ./build/bin/sync-paxos --sync=1 --clockSyncError=${j}ns --boundedMessageDelay=${i}us

       # move the result to the result folder
        mv *.dat result/Sync/Delay_${i}us/Sync_${j}ns/
    done
done

################################################
#        Asynchronous Paxos
################################################

# Change parameters and run the program
# Message Delay
delay_arr=(50000 5000 500 50 5)

mkdir -p result/Async

for i in "${delay_arr[@]}"
do
    mkdir -p result/Async/Delay_${i}us
    
    ./build/bin/sync-paxos --sync=0 --linkDelay=${i}us 

    # move the result to the result folder
    mv *.dat result/Async/Delay_${i}us/
done
