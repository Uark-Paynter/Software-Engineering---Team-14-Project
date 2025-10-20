#include <iostream>
#include <ostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include "UDPBroadcaster.h"
#include "UDPReceiver.h"

int main(int argc, char* argv[])
{
    int mode = atoi(argv[1]);
	std::string N_A = argv[2];
	const char* NA = N_A.c_str();
    
    //Gamemaster Mode
    if (mode == 1) {
		
        //Game Start
		unsigned int second = 1000000;
		std::cout << std::endl << "Starting Game ";
		usleep(second);
		std::cout << "... " << std::flush;
		usleep(second);
		std::cout << "... " << std::flush;
		usleep(second);
		std::cout << "... " << std::flush;
		usleep(second);
		std::cout << "... " << std::flush << std::endl;
		usleep(second);
        Broadcast("202", NA);
		std::cout << "Game Started!" << std::endl;
		
        
		//Game Hold
		usleep (60 * second);
		std::cout << std::endl << "Ending Game..." << std::endl;
		std::cout << "Closing Traffic Gen..." << std::endl;
		Broadcast("221", NA);
		Broadcast("221", NA);
		Broadcast("221", NA);
		usleep (5 * second);
		std::cout << "Closing Receiver..." << std::endl;
		DirectBroadcast (7501, "303", NA);
		DirectBroadcast (7501, "303", NA);
		DirectBroadcast (7501, "303", NA);
		std::cout << "Game Ended!" << std::endl;
    
    }
    
    //Receiver
    if (mode == 2) {
        Receive(NA);
    }
    return 0;
}