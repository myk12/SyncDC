# Change parameters and run the program
#!/bin/bash

# First compile the program
mkdir -p build && cd build && cmake .. && make
cd ../

# Message Size Array: 1MB, 2MB, 4MB, 8MB, 16MB
msg_size_arr=("1MB" "2MB" "4MB" "8MB" "16MB")
mkdir -p result-allreduce/spine-leaf

for i in "${msg_size_arr[@]}"
do
    result_dir="result-allreduce/spine-leaf/msgsize_${i}"
    # Check if the result directory exists, if not create it
    if [ ! -d "$result_dir" ]; then
        mkdir -p "$result_dir"
    fi
    # Run in parallel
    echo "Running Allreduce with message size: ${i}"
    ./build/bin/ml-allreduce --testbed=0 --msgSize=${i} --logDir=${result_dir} &
done

# Fastpass Allreduce
    # Fastpass Allreduce
    # Change parameters and run the program
    # TODO: Implement Fastpass Allreduce logic here