#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define BPORT 7500
#define RPORT 7501

void Broadcast(const char* broadcast_message, const char* NETWORK_ADDRESS) {
    //UDP BROADCASTER
    {
        int sockfdB;
        struct sockaddr_in broadcast_addr;
        
        //Broadcast Socket
        sockfdB = socket(AF_INET, SOCK_DGRAM, 0);
        if(sockfdB == -1) {
            std::cout << "Broadcast Socket Creation Failure" << std::endl;
            exit(-1);
        }
        
        //Broadcast Enable
        int broadcast_enabled = 1;
        if (setsockopt(sockfdB, SOL_SOCKET, SO_BROADCAST, &broadcast_enabled, sizeof(broadcast_enabled)) == -1) {
            std::cout << "Broadcast Enable Failure" << std::endl;
            exit(-1);
        }
        
        //Broadcast Structures
        memset(&broadcast_addr, 0, sizeof(broadcast_addr));
        broadcast_addr.sin_family = AF_INET;
        broadcast_addr.sin_port = htons(BPORT);
        if (inet_pton(AF_INET, NETWORK_ADDRESS, &broadcast_addr.sin_addr) <= 0) {
            std::cout << "INET Error" << std::endl;
            exit(-1);
        }
        
        ssize_t bytes_sent = sendto(sockfdB, broadcast_message, strlen(broadcast_message), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
        if (bytes_sent == -1) {
            std::cout << "Broadcast Failure" << std::endl;
        }
        else {
            std::cout << "Sent: " << bytes_sent << " bytes to: " << NETWORK_ADDRESS << " // Port: " << BPORT << std::endl;
        }
        
        close(sockfdB);
    }
}