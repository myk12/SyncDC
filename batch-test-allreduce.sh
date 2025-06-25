#!/bin/bash

# First compile the program
mkdir -p build && cd build && cmake .. && make
cd ../

# Check if executable build/bin/ml-allreduce exists
if [ ! -f build/bin/ml-allreduce ]; then
    echo "Error: build/bin/ml-allreduce does not exist"
    exit 1
fi

# The result will be save in "result-allreduce" folder
if [ -d result-allreduce ]; then
    rm -rf result-allreduce
    mkdir result-allreduce
fi

################################################
#        Allreduce
################################################

# Testbed: 0-Spine-Leaf, 1-OCS, 2-Fastpass

# OCS Allreduce
# Change parameters and run the program
# Message Size Array: 1MB, 2MB, 4MB, 8MB, 16MB
msg_size_arr=("1MB" "2MB" "4MB" "8MB" "16MB")
sync_error="10ns"
reconf_time="100us"
mkdir -p result-allreduce/ocs-msgsize
for i in "${msg_size_arr[@]}"
do
    mkdir -p result-allreduce/ocs-msgsize/msgsize_${i}
    ./build/bin/ml-allreduce --testbed=1 --msgSize=${i} --syncErrorTime=${sync_error} --reconfTime=${reconf_time}
    # move the result to the result folder
    mv *.log result-allreduce/ocs-msgsize/msgsize_${i}/
done

# Reconfiguration time & Synchronization Error
# Change parameters and run the program
# Reconfiguration Time Array: 50us, 100us, 500us, 1000us
reconf_time_arr=("50us" "100us" "500us" "1000us")
# Synchronization Error Array: 5ns, 50ns, 500ns, 5000ns
sync_error_arr=("5ns" "50ns" "500ns" "5000ns")
mkdir -p result-allreduce/ocs-reconf-sync
for i in "${reconf_time_arr[@]}"
do
    for j in "${sync_error_arr[@]}"
    do
        mkdir -p result-allreduce/ocs-reconf-sync/reconf_${i}/sync_${j}
        ./build/bin/ml-allreduce --testbed=1 --reconfTime=${i} --syncErrorTime=${j}
        # move the result to the result folder
        mv *.log result-allreduce/ocs-reconf-sync/reconf_${i}/sync_${j}/
    done
done

# Spine-Leaf Allreduce
# Change parameters and run the program

# Message Size Array: 1MB, 2MB, 4MB, 8MB, 16MB
msg_size_arr=("1MB" "2MB" "4MB" "8MB" "16MB")
mkdir -p result-allreduce/spine-leaf

for i in "${msg_size_arr[@]}"
do
    mkdir -p result-allreduce/spine-leaf/msgsize_${i}
    # go to the result folder and run the program
    ./build/bin/ml-allreduce --testbed=0 --msgSize=${i}
    # move the result to the result folder
    mv *.log result-allreduce/spine-leaf/msgsize_${i}/
done

# Fastpass Allreduce
# Change parameters and run the program
# TODO
