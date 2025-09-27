#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 7501

int main()
{
    int sockfd;
    struct sockaddr_in my_addr;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    char buffer[1024];
    
    //Set-Up Socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        std::cout << "Socket Creation Failure" << std::endl;
        exit(1);
    }
    
    //Server Struct
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY; //Allows for all-network listening
    
    //Bind Socket -> Port/Address
    if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) {
        std::cout << "Binding Error" << std::endl;
        exit(1);
    }
    
    std::cout << "Listening for UDP Broadcast on: " << PORT << std::endl;
    
    //Data Reception
    while(1) {
        ssize_t num_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &addr_len);
        if (num_bytes == -1) {
            std::cout << "Reception Error" << std::endl;
            exit(1);
        }
        
        //Data Print
        buffer[num_bytes] = '\0';
        std::cout << "Recieved from: " << inet_ntoa(client_addr.sin_addr) << " "  << ntohs(client_addr.sin_port) << " " << buffer << std::endl; 
    }
    
    close(sockfd);
    
    return 0;
}