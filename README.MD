# BaK GL

![GCC 11 Build Status](https://github.com/xavieran/BaKGL/actions/workflows/gcc.yml/badge.svg)
![Clang 13 Build Status](https://github.com/xavieran/BaKGL/actions/workflows/clang.yml/badge.svg)

Remake of Betrayal at Krondor using OpenGL.

# Features
* World exploration
* Towns
* Dialog
* Event state tracking
* Music and sound
* Shops and inventory
* World map

# Acknowledgements

This project would not have been possible without Guido De Jong's original work on xbak (http://xbak.sourceforge.net/).

I have used the late Eysteinn Björnsson's wonderful BaK Help Web extensively as a reference. This website is currently hosted at https://dimwood.net/krondor.html by vga256.

# Requirements
You will need a copy of Betrayal at Krondor for the game data files. These can be found on the web pretty easily.
Requires CMake, SDL2, SDL_MixerX, OpenGL, GLFW, and glm. Many of these dependencies will be downloaded and built automatically.

# Setup
## Extract original data files
In order to play around with the project you will need to extract the original game data files.

This can be done using the "resourcedemo" binary.

In the folder where you have the BaK data files (krondor.001 and krondor.rmf) run

```
$ resourcedemo RMF E
Extracting resources
Created data path: /home/user/capture/
Created data path: /home/user/data/
```

Make sure you check the case of the filenames in the folder where you installed BaK, if they are capitalised you will need to rename or copy them to lowercase.

## Set up shader path
OpenGL shaders are searched for in a specific set of directories

$HOME/shaders
$HOME/.shaders

Either copy the shaders to this directory, or (better) create a symbolic link to point to the shaders directory of the repo.
```
ln -s BakGL/shaders $HOME/shaders
```

## Run main3d
With the files extracted you can run "main3d" in the same directory as the extracted files (e.g. /home/user/data) with an existing save file.
```
$ main3d --save BetrayalAtKrondor/GAMES/TOWN.G06/SAVE02.GAM
```

# Screenshots
![waterfall_ui](screenshots/waterfall_ui.png?raw=true "Waterfall near Tyr-Sog with UI")

![northlands](screenshots/northlands.png?raw=true "Town in Northlands")

![sumani](screenshots/sumani.png?raw=true "Lamut Inn")

![macmordain](screenshots/macmordain.png?raw=true "Mac Mordain Cadall")
