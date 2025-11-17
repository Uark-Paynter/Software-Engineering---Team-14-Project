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
	std::string timeoutinput = argv[3];
	bool timeoutcheck;
	if (timeoutinput == "true")
		timeoutcheck = true;
	if(timeoutinput == "false")
		timeoutcheck = false;
    
    //Gamemaster Mode
    if (mode == 1) {
		
        //Game Start
		unsigned int second = 1000000;
		std::cout << std::endl << "Starting Game ";
		usleep(5*second);
		std::cout << "... " << std::flush;
		usleep(5*second);
		std::cout << "... " << std::flush;
		usleep(5*second);
		std::cout << "... " << std::flush;
		usleep(5*second);
		std::cout << "... " << std::flush << std::endl;
		usleep(10*second);
        Broadcast("202", NA);
		std::cout << "Game Started!" << std::endl;
		
        
		//Game Hold
		usleep (360 * second);
		std::cout << std::endl << "Ending Game..." << std::endl;
		std::cout << "Closing Traffic Gen..." << std::endl;
		Broadcast("221", NA);
		Broadcast("221", NA);
		Broadcast("221", NA);
		std::cout << "Game Ended!" << std::endl;
		std::cout << "[Receiver Port Closing in 45 Seconds] Wait Before Starting New Game, or Close it Manually." << std::endl;
    }
    
    //Receiver
    if (mode == 2) {
        Receive(NA, timeoutcheck);
    }
    return 0;
}