
# BaK GL - _Betrayal at Krondor_ remake using OpenGL

![GCC 13 Build Status](https://github.com/xavieran/BaKGL/actions/workflows/gcc.yml/badge.svg)
![Clang 17 Build Status](https://github.com/xavieran/BaKGL/actions/workflows/clang.yml/badge.svg)

This project is my attempt to remake Betrayal at Krondor using OpenGL.

It started out as a fun project to learn about graphics programming, but I've spent the vast majority of the time reverse engineering and reimplementing the game mechanics instead!

The game is mostly playable (with many rough edges), excluding combat. Improving the graphics will probably land at the end of the roadmap at this point as matching as close as possible the original game behaviour is much more addictive.

Here's a very quick look at some gameplay.

![video](https://github.com/xavieran/blog/blob/master/resources/video/intro_film.gif?raw=true "Intro Video")
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

## To do
* Combat
* Movement clipping
* Lots of polishing!

## Acknowledgements

This project would not have been possible without Guido de Jong's original work on xbak (http://xbak.sourceforge.net/).

I have used the late Eysteinn Björnsson's wonderful BaK Help Web extensively as a reference. This website is currently hosted at https://dimwood.net/krondor.html by vga256. I've also archived the contents [on github](https://github.com/xavieran/BaKHelpWeb).

## Requirements
You will need a copy of Betrayal at Krondor for the game data files. These can be found on the web pretty easily.

Requires CMake, SDL2, SDL_MixerX, OpenGL, GLFW, and glm. Most of these dependencies will be downloaded and built automatically.

I use some features of C++23, so a more recent gcc or clang will be required to compile the code. You can check the CI to see what versions I am currently using.
![GCC 13 Build Status](https://github.com/xavieran/BaKGL/actions/workflows/gcc.yml/badge.svg)
![Clang 17 Build Status](https://github.com/xavieran/BaKGL/actions/workflows/clang.yml/badge.svg)

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
You can build on Windows by opening the project in Visual Studio.

You may need to manually download some of the dependencies (freeglut, glew, and SDL2) and modify the CMakeLists.txt to point to them.

### Mac
Due to Apple's deprecation of OpenGL Mac is not currently supported. It may compile and run if the required dependencies are installed and with some massaging of the CMake files.

## Setup
### BaKGL file main directory
BaKGL needs to find a number of different files in order to run with the game's original data. These are searched for in the _bak main directory_.

On UNIX systems this will be `$HOME/bak`, under Windows it will be `%APPDATA%\bak`. I will hereafter refer to this directory as `$BAK_MAIN_DIR` for both platforms.

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

Save files will be searched for in the `$BAK_MAIN_DIR/GAMES` directory, you can _copy_ your old saves directory to this location. Alternatively, if you have no saves, the game will create this folder when you save your first game.

## Run main3d
The main application is `main3d`. You can run this in the terminal in linux, or select it as the target in Visual Studio.

```
build $ ./main3d
```

Start a new game from the main menu as normal, and enjoy :)

## Config File

You can specify a config file to customise BaKGL. By default it will be loaded from `$BAK_MAIN_DIR/config.json`. You can also specify a config file by command line:
```
$ ./main3d -c path/to/config.json
```

An example `config.json` is included in the main directory of the repo.

## Modding

BaKGL supports replacing the original game image files with your own.

### Set up the Mod Directory
In the `config.json` file, specify the path to your mod directory using the `/Paths/GraphicsOverrides` key. This directory should contain your custom graphics files in PNG format.

### File Replacement
The game will look in this mod directory for files that match the names of the original game assets. For example, if the game would normally load `ACT002.BMX`, you can place a replacement file, called `ACT002.PNG`, in the graphics overrides directory. The game will use your custom file instead of the original one. You should make sure your replacement file has the same aspect ratio as the original file or it will look stretched when rendered in game.

### Handling Multi Image Files
Some BMX files may contain multiple images, for example the inventory icons file (`INVSHP0.BMX`). For these, the game will look for a subdirectory named after the base file and use an indexed image.
If the original file is `INVSHP0.BMX` and the game needs the first image , it will search for a file named `INVSHP0.BMX/0.PNG` in your graphics mod directory.

### Exporting all images from the game
I've provided an application to export all images from the game into a directory structure suitable for use as a template for modding. You can run it like so:
```
build $ ./image_extractor -r krondor.001 -i krondor.rmf -o mod_dir
```

You can then use this as the basis for graphical mods. Use this directory as the `/Paths/simply replace the PNG files in this directory structure with higher resolution ones as you make them.

# Screenshots
![waterfall_ui](screenshots/waterfall_ui.png?raw=true "Waterfall near Tyr-Sog with UI")

![northlands](screenshots/northlands.png?raw=true "Town in Northlands")

![sumani](screenshots/sumani.png?raw=true "Lamut Inn")

![macmordain](screenshots/macmordain.png?raw=true "Mac Mordain Cadall")
