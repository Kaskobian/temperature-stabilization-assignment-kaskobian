#!/bin/bash

# Check for correct number of arguments
if [ "$#" -ne 5 ]; then
    echo "Usage: $0 <central_temp> <client1_temp> <client2_temp> <client3_temp> <client4_temp>"
    exit 1
fi

CENTRAL_TEMP=$1
CLIENT1_TEMP=$2
CLIENT2_TEMP=$3
CLIENT3_TEMP=$4
CLIENT4_TEMP=$5

# Compile server and client
echo "Compiling server and client..."
gcc -o server tcp_server.c utils.c -Wall
gcc -o client tcp_client.c utils.c -Wall

if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

echo "Compilation successful."

# Run server in the current terminal
echo "Starting server with initial central temp: $CENTRAL_TEMP"
gnome-terminal -- bash -c "./server $CENTRAL_TEMP; exec bash"

# Sleep a bit to give server time to start
sleep 2

# Run clients in separate terminals
echo "Starting clients..."
gnome-terminal -- bash -c "./client 1 $CLIENT1_TEMP; exec bash"
gnome-terminal -- bash -c "./client 2 $CLIENT2_TEMP; exec bash"
gnome-terminal -- bash -c "./client 3 $CLIENT3_TEMP; exec bash"
gnome-terminal -- bash -c "./client 4 $CLIENT4_TEMP; exec bash"

echo "All processes started."
