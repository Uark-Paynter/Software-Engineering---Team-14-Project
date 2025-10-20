#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
//#include "Scorecard.h"

#define BPORT 7500
#define RPORT 7501
    
void Receive(const char* NETWORK_ADDRESS) {
    //UDP RECEIVER
    {
        int sockfdR;
        struct sockaddr_in my_addr;
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(struct sockaddr_in);
        char buffer[1024];
        
        //Receiver Socket
        if ((sockfdR = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            std::cout << "Receiver Socket Creation Failure" << std::endl;
            exit(-1);
        }
        
        //Receiver Structures
        memset(&my_addr, 0, sizeof(my_addr));
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(RPORT);
        my_addr.sin_addr.s_addr = INADDR_ANY;
        
        //Receiver Binding
        if (bind(sockfdR, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) {
            std::cout << "Receiver Binding Error" << std::endl;
            exit(-1);
        }
        
        std::cout << "Listening for UDP Broadcast on: " << RPORT << std::endl;
        
        //Data Reception
        while(1) {
            ssize_t num_bytes = recvfrom(sockfdR, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &addr_len);
            if (num_bytes == -1) {
                std::cout << "Reception Error" << std::endl;
            }
            
            //Data Print
            buffer[num_bytes] = '\0';
            std::cout << "Received from: " << inet_ntoa(client_addr.sin_addr) << " " << ntohs(client_addr.sin_port) << " [" << buffer << "]" << std::endl;
            
			//Receiver Closer
			if ((buffer[0] == '3') && (buffer[1] == '0') && (buffer[2] == '3')) {
				std::cout << "Received Closer Signal" << std::endl;
				exit(1);
			}
			
            //Data Hit Broadcast
                //Broadcast hit id (C: x:int)
                char breaker  = ':';
                int breaknum = 0;
                char hit[3] = "";
                char tagger[3] = "";
                
                //Find ":"
                for (int i = 0; buffer[i] != '\0'; i++) {
                    if (buffer[i] == breaker) {
                        breaknum = i;
                    }
                }
                
                //Break Hit/Tag
                for (int i = 0; i < breaknum; i++) {
					//std::cout << "Break: " << breaknum << " // Tag: " << tagger[i] << " Buffer: " << buffer[i] << std::endl;
                    tagger[i] = buffer[i];
                }
                std::cout << "Tag: " << tagger;
                
                for (int i = breaknum+1; buffer[i] != '\0'; i++) {
					//std::cout << "Break: " << breaknum << " // Hit: " << hit[i-breaknum-1] << " Buffer: " << buffer[i] << std::endl;
                    hit[i-breaknum-1] = buffer[i];
                }
                std::cout << " // Hit: " << hit << std::endl;
                

            //Tag Handler
            
                //P2P Tagging (C: int:int)
                if ((atoi(hit) < 40) && (atoi(tagger) < 40)) {
					std::cout << "P2P ";
                    //Enemy Tag (C: mod != mod)
					if ((atoi(hit) % 2) != (atoi(tagger) % 2)) {
						std::cout << "Enemy Tag" << std::endl;
						//updateScore(tagger, 10);
						//updateScore(hit, -10);
					}
                    //Friendly Tag (C: mod = mod)
                    if ((atoi(hit) % 2) == (atoi(tagger) % 2)) {
						std::cout << "Friendly Tag" << std::endl;
						Broadcast(tagger, NETWORK_ADDRESS);
						//updateScore(tagger, 10);
						//updateScore(hit, -10);
					}
				}
                //Red Base Tagging (C: int:53)
                if (atoi(hit) == 43) {
					std::cout << "Red Base Tagged" << std::endl;
					//updateScore(tagger, 100);
				}
                //Green Base Tagging (C: int:43)
				if (atoi(hit) == 53) {
					std::cout << "Green Base Tagged" << std::endl;
					//updateScore(tagger, 100);
				}
            
			//Broadcast HitID
				Broadcast(hit, NETWORK_ADDRESS);
				std::cout << std::endl;
        }
        
        close(sockfdR);
        
    }
}