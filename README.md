[NOTICE:]
View as raw for better formatting.
-Depends on: QT installation, postgreSQL, Working VM (Cannot run on Windows environment), Python traffic generator (Photon github) for simulation

To run from terminal in VM:
- chmod u+x build.sh
- ./build.sh

[NOTICE:]
If for some reason something is not working, manual install and compile instructions are preserved below. (Ignore leading "-")
Install dependencies from terminal:
- sudo apt update
- sudo apt install libpq-dev
- sudo apt install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
- sudo apt install libsdl2-dev libsdl2-mixer-dev


[NOTICE:] 
Please compile (Game.cpp) and (randomtrack.cpp) before trying to start the game from (main.cpp).

Game:
- g++ Game.cpp Scorecard.cpp -o Game.exe -lrt -pthread
- ./Game.exe

Music Player:
- g++ randomtrack.cpp -o randomtrack `sdl2-config --cflags --libs` -lSDL2_mixer

Main Program:
- g++ -fPIC main.cpp Scorecard.cpp -o LaserTagApp '$(pkg-config --cflags --libs Qt5Widgets)' -lpq -pthread -lrt
- ./LaserTagApp

[Credits]
UI and Windows for the project were made by Dylan
Database Connections, Music Player, and Build script were mady by Bryan
Database Connections, Functional Keys, and Misc Functions were made by Jeremiah
UDP Socket Programs and Scoreboard Systems were made by Logan
