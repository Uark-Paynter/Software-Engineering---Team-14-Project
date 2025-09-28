#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>


int main()
{
    int PORT = 0;
    std::string NETWORK_ADDRESS = "127.0.0.1";
    std::cout << "Enter Port:" << std::endl;
    std::cin >> PORT;
    std::cout << "Enter Network Address (""a"" for Default: 127.0.0.1):" << std::endl;
    std::cin >> NETWORK_ADDRESS;
	if(NETWORK_ADDRESS == "a")
		NETWORK_ADDRESS = "127.0.0.1";
    
    int sockfd;
    struct sockaddr_in broadcast_addr;
    char broadcast_message[] = "UDP Broadcast Test";
    
    std::cout << "Enter message:" << std::endl;
    std::cin.ignore();
    std::cin.getline(broadcast_message, 1000);
    
    //Set-Up Socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        std::cout << "Socket Creation Failed" << std::endl;
        exit(1);
    }
    
    //Allow Broadcasting
    int broadcast_enabled = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enabled, sizeof(broadcast_enabled)) == -1) {
        std::cout << "Enable Broadcast Failure" << std::endl;
        exit(1);
    }
    
    //Create & Initialize Broadcast Structures
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, NETWORK_ADDRESS.c_str(), &broadcast_addr.sin_addr) <= 0) {
        std::cout << "inet Error" << std::endl;
        exit(1);
    }
    
    //Send Msg
    ssize_t bytes_sent = sendto(sockfd, broadcast_message, strlen(broadcast_message), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    
    if (bytes_sent == -1) {
        std::cout << "Send Error" << std::endl;
    }
    else {
        std::cout << "Sent " << bytes_sent << " to: " << NETWORK_ADDRESS << " Port: " << PORT << std::endl;
    }
    
    close(sockfd);
    
    return 0;
}