#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

bool cmpAddr(sockaddr_in a, sockaddr_in b) {
    return true;
}

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("Wrong argv srcport dstport\n");
        return -1;
    }
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0) {
        printf("Error socket\n");
        return -1;
    }
    sockaddr_in serverAddr;
    sockaddr_in listenAddr;
    sockaddr_in clientAddr;
    sockaddr_in tempAddr;
    char buffer[10240];
    ssize_t len;
    socklen_t addr_size;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "10.128.171.151", &serverAddr.sin_addr);
    listenAddr = serverAddr;
    serverAddr.sin_port = htons(atoi(argv[2]));
    listenAddr.sin_port = htons(atoi(argv[1]));
    if(bind(socket_fd, (sockaddr*) &listenAddr, sizeof(sockaddr_in)) < 0) {
        printf("Error bind\n");
        return -1;
    }
    while(true) {
        len = recvfrom(socket_fd, buffer, 10240, 0, (sockaddr*)&tempAddr, &addr_size);
        if(len >= 0) {
            printf("transmit ");
            if(memcmp(&tempAddr, &serverAddr, sizeof(sockaddr)) == 0) {
                printf("to client\n");
                sendto(socket_fd, buffer, len, 0, (sockaddr*) &clientAddr, sizeof(sockaddr));
            }
            else {
                clientAddr = tempAddr;
                printf("to server\n");
                sendto(socket_fd, buffer, len, 0, (sockaddr*) &serverAddr, sizeof(sockaddr));
            }
        }
    }
    return true;
}
