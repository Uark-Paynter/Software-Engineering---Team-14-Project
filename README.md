[NOTICE:] View as raw for better formatting.

Files for Sprint 2 from Logan
-Depends on: QT installation, Working VM (Cannot run on Windows environment), Python traffic generator (Photon github) for simulation

[NOTICE:] All of these files are meant for submission. Please ensure that all changes made to Dylan's program also reflect those made in my edit here. I have clearly labelled anything I edited in this version.
[NOTICE:] I am including .exe files as well. If changes are made, please recompile before running the programs. Instructions for terminal compilation and run commands are below.
[NOTICE:] Images for the GUI will intentionally not load from this branch alone. Please include the images folder provided by Dylan in the same directory as the program.
This includes raw .cpp files for:
- UDP Broadcast & Receiver (UDPBroadcaster) (UDPReceiver)
- Gamemaster Program (Game.cpp)
- An updated version of Dylan's GUI program that interacts and calls my sockets (GUI.cpp)

This also includes additonal support files that ARE NOT meant to be turned in with Sprint 2:
These files are meant to help debug and test the UDP sockets and program as a whole.
- Broadcast spoofer (Allows for direct broadcasts to be sent from user input port, network address, and with custom message) (support/broadcast-spoofer.cpp)
- UDP Receivers for debugging on Ports 7500, 7501. (support/receiver-7500) (support/receiver-7501)
- A copy of the Python traffic generator (support/ptg.py)

[NOTICE:] Please compile (Game.cpp) before trying to start the game from (GUI.cpp).
To run from terminal in VM:

Game:
  g++ Game.cpp -o Game.cpp

GUI:
  g++ -fPIC GUI.cpp -o GUI.exe `pkg-config --cflags --libs Qt5Widgets`
  ./GUI.exe


Python Traffic Generator
  python3 ptg.py

Misc Support files:
  g++ filename.cpp -o filename.exe
  ./filename.exe


For db access:
-making sure the pqsl libraries are there
sudo apt update
sudo apt install libpq-dev
- in case the normal command lines above don't work correctly run with the -get
sudo apt-get update 
sudo apt-get install libpq-dev
-for qt
sudo apt update
sudo apt install qtbase5-dev

-then to get the application on the folder to appear:
g++ main3.cpp -o LaserTagApp -fPIC     $(pkg-config --cflags --libs Qt5Widgets) -lpq
-finally to open the application:
./LaserTagApp
  

  
