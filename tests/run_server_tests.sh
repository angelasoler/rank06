#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Use: ./run_server_tests.sh <PORT>"
    exit 1
fi

PORT=$1

# Runs server on detach mode
./server $PORT &
SERVER_PID=$!

# Wait for server to start
sleep 1


echo "========= test =========="

diff -U 3 client1_output.txt expected_output_client1.txt > client1_trace.txt
if [ -s client1_trace.txt ]; then
    echo -e "\033[31mFail\033[0m"
    cat client1_trace.txt
else
    echo -e "\033[32mSucced\033[0m"
fi

diff -U 3 client0_output.txt expected_output_client0.txt > client0_trace.txt
if [ -s client_trace.txt ]; then
    echo -e "\033[31mFail\033[0m"
    cat client0_trace.txt
else
    echo -e "\033[32mSucced\033[0m"
fi


sleep 1

kill $SERVER_PID
