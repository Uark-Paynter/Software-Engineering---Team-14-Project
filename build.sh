#!/bin/bash
#to be able to run
chmod +x "$0"
if command -v dos2unix >/dev/null 2>&1; then
    dos2unix "$0" >/dev/null 2>&1
fi


set -e #if commands fail

echo "Updating packages"
sudo apt update -y

echo "Installing PostgreSQL + Qt5"
sudo apt install -y libpq-dev qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools libsdl2-dev libsdl2-mixer-dev

echo "Compiling Game.cpp"
g++ Game.cpp Scorecard.cpp -o Game.exe -lrt -pthread 

echo "Compiling randomtrack.cpp"
g++ randomtrack.cpp -o randomtrack `sdl2-config --cflags --libs` -lSDL2_mixer

echo "Compiling main.cpp"
g++ -fPIC main.cpp Scorecard.cpp -o LaserTagApp $(pkg-config --cflags --libs Qt5Widgets) -lpq -pthread -lrt

echo "Running LaserTagApp"
./LaserTagApp