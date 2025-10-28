#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include "utils.h"

int main(int argc, char *argv[])
{
    if(argc < 3){
        printf("Usage: %s <external index> <initial temperature>\n", argv[0]);
        return -1;
    }

    int externalIndex = atoi(argv[1]);
    float externalTemp = atof(argv[2]);

    int socket_desc;
    struct sockaddr_in server_addr;
    struct msg the_message;

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc < 0){ perror("socket"); return -1; }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("connect"); return -1;
    }

    printf("Client %d connected to server\n", externalIndex);

    while(1){
        // Send current temperature to server
        the_message = prepare_message(externalIndex, externalTemp);
        if(send(socket_desc, &the_message, sizeof(the_message), 0) <= 0){
            perror("send"); break;
        }

        // Receive updated temperature from server
        if(recv(socket_desc, &the_message, sizeof(the_message), 0) <= 0){
            perror("recv"); break;
        }

        // Check for done signal
        if(the_message.T == -1){
            printf("Client %d: System stabilized. Final Temp = %.6f\n", externalIndex, externalTemp);
            break;
        }

        float centralTemp = the_message.T;

        // Update external temperature
        externalTemp = (3*externalTemp + 2*centralTemp)/5.0;

        printf("Client %d: Updated Temperature = %.6f\n", externalIndex, externalTemp);
    }

    close(socket_desc);
    return 0;
}
