# BaK GL

![GCC 11 Build Status](https://github.com/xavieran/BaKGL/actions/workflows/gcc.yml/badge.svg)
![Clang 13 Build Status](https://github.com/xavieran/BaKGL/actions/workflows/clang.yml/badge.svg)

Reimplementation of Betrayal at Krondor using OpenGL for true 3D graphics.

Main goals are to get the game running (graphics, world exploration, story, and eventually combat).

Reuses much of the original work of xbak (http://xbak.sourceforge.net/) but I am rewriting/replacing xbak functionality as necessary.

# Requirements
You will need a copy of Betrayal at Krondor for the game data files. These can be found on the web pretty easily.
Requires CMake, SDL (xbak dependency), OpenGL, GLFW, and glm.

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

![waterfall](screenshots/waterfall.png?raw=true "Waterfall near Tyr-Sog")

![macmordain](screenshots/macmordain.png?raw=true "Mac Mordain Cadall")
