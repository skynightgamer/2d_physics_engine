# 2d_physics_engine
A simple 2D physics engine for a school project


## Installation

Windows school computer:

follow this guide: https://code.visualstudio.com/docs/cpp/config-mingw until adding the PATH variable.

dont set up the env variable as this can not be done (not enough rights)

then install git using pacman:

pacman -S git


then proceed to navigate to  /c/Users/${login_name} and git clone the repo:

git clone https://github.com/skynightgamer/2d_physics_engine.git

navigate to the project folder,

then in the same msys2 shell, compile and run the program:

g++ main.cpp

./a.exe



## macOS

g++ main.cpp -I/opt/homebrew/opt/sfml@2/include \
             -L/opt/homebrew/opt/sfml@2/lib \
             -lsfml-graphics -lsfml-window -lsfml-system


