#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <math.h>
#include "utils.h"

#define numExternals 4
#define EPS 1e-3

int * establishConnectionsFromExternalProcesses()
{
    int socket_desc;
    static int client_socket[numExternals]; 
    unsigned int client_size;
    struct sockaddr_in server_addr, client_addr;

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc < 0){ perror("socket"); exit(1); }

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){ perror("bind"); exit(1); }

    // Listen:
    if(listen(socket_desc, numExternals) < 0){ perror("listen"); exit(1); }

    printf("Listening for incoming connections...\n");

    // Accept 4 clients
    for(int i = 0; i < numExternals; i++){
        client_size = sizeof(client_addr);
        client_socket[i] = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);
        if(client_socket[i] < 0){ perror("accept"); exit(1); }
        printf("External process %d connected from IP: %s, port: %d\n", i+1,
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    printf("All external processes connected.\n");
    return client_socket;
}

int main(int argc, char *argv[])
{
    if(argc < 2){
        printf("Usage: %s <initial central temperature>\n", argv[0]);
        return -1;
    }

    float centralTemp = atof(argv[1]);
    struct msg messageFromClient;
    int *client_socket = establishConnectionsFromExternalProcesses();

    float oldTemps[numExternals] = {0};
    float externalTemps[numExternals];

    bool stable = false;
    while(!stable){
        // Receive temperatures from clients
        for(int i=0; i<numExternals; i++){
            if(recv(client_socket[i], &messageFromClient, sizeof(messageFromClient), 0) <= 0){
                perror("recv");
                exit(1);
            }
            externalTemps[i] = messageFromClient.T;
            printf("Received from External %d: %.6f\n", i+1, externalTemps[i]);
        }

        // Update central temperature
        float sum = 0;
        for(int i=0;i<numExternals;i++) sum += externalTemps[i];
        centralTemp = (2*centralTemp + sum)/6.0;
        printf("Updated Central Temp = %.6f\n", centralTemp);

        // Update external temperatures using formula
        for(int i=0;i<numExternals;i++){
            externalTemps[i] = (3*externalTemps[i] + 2*centralTemp)/5.0;
        }

        // Send updated temperatures to clients
        for(int i=0;i<numExternals;i++){
            messageFromClient = prepare_message(0, externalTemps[i]);
            if(send(client_socket[i], &messageFromClient, sizeof(messageFromClient), 0) <= 0){
                perror("send");
                exit(1);
            }
        }

        // Check stability
        stable = true;
        for(int i=0;i<numExternals;i++){
            if(fabs(externalTemps[i] - oldTemps[i]) > EPS){
                stable = false;
            }
            oldTemps[i] = externalTemps[i];
        }
    }

    // Send "done" signal to clients (T = -1)
    for(int i=0;i<numExternals;i++){
        messageFromClient = prepare_message(0, -1);
        send(client_socket[i], &messageFromClient, sizeof(messageFromClient), 0);
        close(client_socket[i]);
    }

    printf("System stabilized. Final Central Temp = %.6f\n", centralTemp);
    return 0;
}
