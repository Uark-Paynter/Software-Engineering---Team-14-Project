[NOTICE:]
View as raw for better formatting.
-Depends on: QT installation, postgreSQL, Working VM (Cannot run on Windows environment), Python traffic generator (Photon github) for simulation

-Install QT and postgreSQL from terminal:
-sudo apt update
-sudo apt install libpq-dev
-sudo apt install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools

This includes raw .cpp files for:
- UDP Broadcast & Receiver (UDPBroadcaster) (UDPReceiver)
- Gamemaster Program (Game.cpp)
- GUI program that interacts and calls sockets (main.cpp)

[NOTICE:] 
Please compile (Game.cpp) before trying to start the game from (main.cpp).

To run from terminal in VM:
Game:
  g++ Game.cpp -o Game.exe
  ./Game.exe

GUI (Main Program):
  g++ -fPIC main.cpp -o LaserTagApp `pkg-config --cflags --libs Qt5Widgets` -lpq
  ./LaserTagApp

[Credits]
UI for this project was made by Dylan.
Database connection and connections were made by Bryan and Jeremiah.
UDP socket programs were made by Logan.
