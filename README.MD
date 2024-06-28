
# BaK GL - _Betrayal at Krondor Remake in OpenGL_

![GCC 11 Build Status](https://github.com/xavieran/BaKGL/actions/workflows/gcc.yml/badge.svg)
![Clang 16 Build Status](https://github.com/xavieran/BaKGL/actions/workflows/clang.yml/badge.svg)

This project is my attempt to remake Betrayal at Krondor using OpenGL. It started out as a fun project to learn about graphics programming, but to be honest, I've spent the fast majority of the time reverse engineering and reimplementing the game mechanics instead! The game is mostly playable (with many rough edges), excluding combat. Improving the graphics will probably land at the end of the roadmap at this point as matching as close as possible the original game behaviour is much more addictive.

## Features
* Camping
* Chapter progression
* Cutscenes/Book excerpts
* Music and sound
* Quests and story
* Shops and inventory
* Spell casting
* Temples
* Towns
* World exploration
* World map

## Acknowledgements

This project would not have been possible without Guido De Jong's original work on xbak (http://xbak.sourceforge.net/).

I have used the late Eysteinn Björnsson's wonderful BaK Help Web extensively as a reference. This website is currently hosted at https://dimwood.net/krondor.html by vga256.

## Requirements
You will need a copy of Betrayal at Krondor for the game data files. These can be found on the web pretty easily.
Requires CMake, SDL2, SDL_MixerX, OpenGL, GLFW, and glm. Most of these dependencies will be downloaded and built automatically.

## Build
### Linux
This project uses cmake. It can be built easily with
```
mkdir build; cd build;
cmake ..
make
```

The relevant binaries will be placed in your build directory.

### Windows
Alternatively you can build on Windows by opening the project in Visual Studio.

You may need to manually download some of the dependencies (freeglut, glew, and SDL2) and modify the CMakeLists.txt to point to them.

### Mac
Due to Apple's deprecation of OpenGL Mac is not currently supported. It may work on earlier versions, I've had some success using brew to install dependencies.

## Setup
### BaKGL file main directory
BaKGL needs to find a number of different files in order to run with the games original data. These are searched for in the "bak main directory".

On UNIX systems this will be "$HOME/bak", under Windows it will be "%APPDATA%\bak". I will hereafter refer to this directory as `$BAK_MAIN_DIR` for both platforms.

### Locate the original data files
In order to play around with the project you will need the original game data files.

You will specifically need the following files from your game: 
```
krondor.001
krondor.rmf
frp.sx
startup.gam
```

BaKGL will look in either the current directory from which you ran the executable, or `$BAK_MAIN_DIR/data` for the game data files, so please copy them to the appropriate folder.

Make sure you check the case of the filenames in the folder where you installed BaK, if they are capitalised you will need to rename or copy them to lowercase.

Save files will be searched for in the `$BAK_MAIN_DIR/saves/GAMES` directory, you can _copy_ your old saves directory to this location.

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

Start a new game from the main menu as normal, and enjoy :)

# Screenshots
![waterfall_ui](screenshots/waterfall_ui.png?raw=true "Waterfall near Tyr-Sog with UI")

![northlands](screenshots/northlands.png?raw=true "Town in Northlands")

![sumani](screenshots/sumani.png?raw=true "Lamut Inn")

![macmordain](screenshots/macmordain.png?raw=true "Mac Mordain Cadall")
