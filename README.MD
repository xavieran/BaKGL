# BaK GL

![GCC 11 Build Status](https://github.com/xavieran/BaKGL/actions/workflows/gcc.yml/badge.svg)
![Clang 16 Build Status](https://github.com/xavieran/BaKGL/actions/workflows/clang.yml/badge.svg)

Remake of Betrayal at Krondor using OpenGL.

# Features
* Camping
* Cutscenes
* Music and sound
* Quests and story
* Shops and inventory
* Spell casting
* Temples
* Towns
* World exploration
* World map

# Acknowledgements

This project would not have been possible without Guido De Jong's original work on xbak (http://xbak.sourceforge.net/).

I have used the late Eysteinn Björnsson's wonderful BaK Help Web extensively as a reference. This website is currently hosted at https://dimwood.net/krondor.html by vga256.

# Requirements
You will need a copy of Betrayal at Krondor for the game data files. These can be found on the web pretty easily.
Requires CMake, SDL2, SDL_MixerX, OpenGL, GLFW, and glm. Many of these dependencies will be downloaded and built automatically.

# Build
## Linux
This project uses cmake. It can be built easily with
```
mkdir build; cd build;
cmake ..
make
```

The relevant binaries will be placed in your build directory.

## Windows
Alternatively you can build on Windows by opening the project in Visual Studio.

You may need to manually download some of the dependencies (freeglut, glew, and SDL2) and modify the CMakeLists.txt to point to them.

# Setup
## BaKGL file main directory
BaKGL needs to find a number of different files in order to run with the games original data. These are searched for in the "bak main directory".

On UNIX systems this will be "$HOME/bak", under Windows it will be "%APPDATA%\bak". I will refer to this directory from here as $BAK_MAIN_DIR for both platforms.

## Locate the original data files
In order to play around with the project you will need the original game data files.

You will specifically need the following files from your game: "krondor.001", "krondor.rmf", and "frp.sx".

BaKGL will look in either the current directory from which you ran the executable, or $BAK_MAIN_DIR/data for the game data files, so please copy them to the appropriate folder.

Make sure you check the case of the filenames in the folder where you installed BaK, if they are capitalised you will need to rename or copy them to lowercase.

Save files will be searched for in the $BAK_MAIN_DIR/saves/GAMES directory, you can copy your old saves directory to this location. If you have no saves you will need to run BAK at least once in dosbox in order to create an initial save file.

## Set up shader path
OpenGL shaders are searched for in a specific set of directories

$HOME/shaders
$HOME/.shaders
$BAK_MAIN_DIR/shaders

Either copy the shaders to this directory, or (better) create a symbolic link to point to the shaders directory of the repo.
```
ln -sf BakGL/shaders $BAK_MAIN_DIR/shaders
```

## Run main3d
The main application is "main3d". You can run this in the terminal in linux, or select it as the target in Visual Studio.

```
$ main3d
```

If you do not specify a save file, BaKGL will look in $BAK_MAIN_DIR/save/NEW_GAME.GAM for a save to use as the new game. I usually start a new game in original bak, save without doing anything, and then copy the save file to this location.

# Screenshots
![waterfall_ui](screenshots/waterfall_ui.png?raw=true "Waterfall near Tyr-Sog with UI")

![northlands](screenshots/northlands.png?raw=true "Town in Northlands")

![sumani](screenshots/sumani.png?raw=true "Lamut Inn")

![macmordain](screenshots/macmordain.png?raw=true "Mac Mordain Cadall")
