#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include "Scorecard.h"

#define BPORT 7500
#define RPORT 7501
    
void Receive(const char* NETWORK_ADDRESS, bool timeoutcheck) {
    //UDP RECEIVER
    {
        Scorecard* scorecard = initSharedMemory(false);
        int sockfdR;
        struct sockaddr_in my_addr;
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(struct sockaddr_in);
        char buffer[1024];
		bool redTags[15] = {false};
		bool greenTags[15] = {false};
        
        //Receiver Socket
        if ((sockfdR = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            std::cout << "Receiver Socket Creation Failure" << std::endl;
            exit(-1);
        }
		
		//Timeout
		struct timeval timeout;
		timeout.tv_sec = 45;
		timeout.tv_usec = 0;
		if (timeoutcheck == true) {
			setsockopt(sockfdR, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
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
                std::cout << "Reception or Timeout Error" << std::endl;
				std::cout << "Receiver has a 45 second timeout window" << std::endl;
				std::cout << "This prevents port blocking after game ends" << std::endl;
				std::cout << "Disable this in the main menu" << std::endl;
				exit(-1);
            }
            
            //Data Print
            buffer[num_bytes] = '\0';
            std::cout << "Received from: " << inet_ntoa(client_addr.sin_addr) << " " << ntohs(client_addr.sin_port) << " [" << buffer << "]" << std::endl;
            
            //Data Hit Broadcast
                //Broadcast hit id (C: x:int)
                char breaker  = ':';
                int breaknum = 0;
                char hit[3] = "";
                char tagger[3] = "";
				
				//Event Logger
				char event[16];
				strncpy(event, buffer, sizeof(event)-1);
				event[sizeof(event)-1] = '\0';
				logEvent(scorecard, event);
                
                //Find ":"
                for (int i = 0; buffer[i] != '\0'; i++) {
                    if (buffer[i] == breaker) {
                        breaknum = i;
                    }
                }
                
                //Break Hit/Tag
                for (int i = 0; i < breaknum; i++) {
                    tagger[i] = buffer[i];
                }
                std::cout << "Tag: " << tagger;
                
                for (int i = breaknum+1; buffer[i] != '\0'; i++) {
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
						updateScore(scorecard, atoi(tagger), 10);
						updateScore(scorecard, atoi(hit), -10);
					}
                    //Friendly Tag (C: mod = mod)
                    if ((atoi(hit) % 2) == (atoi(tagger) % 2)) {
						std::cout << "Friendly Tag" << std::endl;
						Broadcast(tagger, NETWORK_ADDRESS);
						updateScore(scorecard, atoi(tagger), -10);
						updateScore(scorecard, atoi(hit), -10);
					}
				}
                //Green Base Tagging by Red (C: int:53)
                if (atoi(hit) == 43) {
					if (atoi(tagger) %2 == 1) {
						std::cout << "Green Base Tagged";
						if (redTags[atoi(tagger)]) {
							std::cout << " Again, [No Points]" << std::endl;
						}
						if (!redTags[atoi(tagger)]) {
							updateScore(scorecard, atoi(tagger), 100);
							redTags[atoi(tagger)] = true;
							std::cout << std::endl;
							//Add base icons
						}
					}
					else {
						std::cout << "Red Base Self-Tag [No Points]" << std::endl;
					}
				}
                //Red Base Tagging by Green (C: int:43)
				if (atoi(hit) == 53) {
					if (atoi(tagger) %2 != 1) {
						std::cout << "Red Base Tagged";
						if (greenTags[atoi(tagger)]) {
							std::cout << " Again, [No Points]" << std::endl;
						}
						if (!greenTags[atoi(tagger)]) {
							updateScore(scorecard, atoi(tagger), 100);
							greenTags[atoi(tagger)] = true;
							std::cout << std::endl;
							//Add base icons
						}
					}
					else {
						std::cout << "Green Base Self-Tag [No Points]" << std::endl;
					}
				}
            
			//Broadcast HitID
				Broadcast(hit, NETWORK_ADDRESS);
				std::cout << std::endl;
        }
        
        close(sockfdR);
        
    }
}