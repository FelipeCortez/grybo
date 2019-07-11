#Grybo

A Guitar Hero clone made with SDL2 and OpenGL.

![Grybo](https://raw.githubusercontent.com/CFelipe/grybo/master/screenshot.png "Grybo")

## Building the project

Currently the Makefile is MAC OS specific. It can be easily ported to Linux.  

## Contributing to the project

CMake build files are welcome, in case you'd like to contribute to this project.

### Installing dependencies

In order to build this project you'll need to install these libraries:

* sdl2
* libsoundio
* assimp

If you have brew installed, this can be easily done by typing the following command:

```
brew install sdl2&&brew install libsoundio&&brew install assimp
```

If you don't have the **brew** package manager installed on your MAC system you can easily install it by executing this command:

```
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" < /dev/null 2> /dev/null
```
