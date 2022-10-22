# Deferred renderer with deferred, depth-peeling transparency

## Build guide

The OpenGL-based project can be built through the [CMake](https://cmake.org) build system in the native OS. The following paragraphs will guide you through the compilation of the project depending on your platform of choice.

### Windows

Pre-requisites:
- [Git Bash](https://gitforwindows.org) (for issuing unix-style commands)
- [MinGW-w64](http://mingw-w64.org) (GCC 11.2.0 or higher)
- [CMake 3.18](https://cmake.org) (or higher)

It is recommended to install MinGW-w64 and CMake through [MSYS2](https://www.msys2.org/) in order to get the latest version of MinGW-w64's GCC and CMake. To do so:
- Download and install MSYS2.
- Run "MSYS2 MSYS" from the start menu; a terminal will appear.
- Run `pacman -Syu`; a restart of all MSYS2 processes is required at the end of the update.
- Run "MSYS2 MSYS" again, and run `pacman -Su`.
- Run `pacman -S --needed base-devel mingw-w64-x86_64-toolchain` to install the MinGW-w64 toolchain.
- Run `pacman -S mingw-w64-x86_64-cmake` to install CMake.
- Run `pacman -S mingw-w64-x86_64-make` to install MinGW-w64's make tool (used by CMake).

After the installation of these packages, make sure to add their path to the `PATH` environment variable (e.g., by editing the `.bashrc` file in your home). They should reside in MSYS2's installation folder as such:
```
C:\msys64\mingw64\bin
```
but the actual path may vary depending on your installation (GCC's and CMake's binaries are already in `PATH` if you execute the "MSYS2 MinGW x64" shortcut from the start menu). Then, you should be able to build the whole project with CMake through:
```
cmake -S ./ -B ./bin -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build ./bin/
```

### Linux

Pre-requisites:
- Xorg-dev package (X11)
- G++ 9 (or higher)
- [CMake 3.18](https://cmake.org) (or higher)

To install these packages on Ubuntu, type the following command:
```
sudo apt-get install xorg-dev g++ cmake
```
Then, you should be able to build the whole project with CMake through:
```
cmake -S ./ -B ./bin -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build ./bin/
```

### MacOS

Pre-requisites:
- Xcode Command Line Tools
- [CMake 3.18](https://cmake.org) (or higher)

To install them, assuming you have the [brew](https://brew.sh) package manager, type the following commands:
```
xcode-select --install
brew install cmake
```
Then, you should be able to build the whole project with CMake through:
```
cmake -S ./ -B ./bin -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build ./bin/
```

## Build tips

There are be some tips that you may find useful while compiling this project with CMake.

### Avoid to re-build heavy libraries

If you're editing source files of your project, you might not want to re-build external libraries again, especially Assimp and Bullet Physics. This might become a time wasting issue while debugging your code.
To prevent this, remember to generate the makefiles only when the structure of the project changes (e.g.: the source list has changed). Therefore, just hit `cmake --build ./bin/` and external libraries shouldn't recompile again.

### Clean builds

When you modify the project structure heavily, it's advised to delete the current `./bin` folder and repeat the build process from the ground up in order to get a clean build. No longer useful files of a previous build (e.g.: an object file of a deleted .cpp file) might be kept in the `./bin` folder.
Yet, this can be avoided if you know what you're doing. For instance, if you incrementally add more files into the source list of the project, the build should still be clean. 