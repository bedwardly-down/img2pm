This is my WIP build of img2pm so that it can be used with the Epson C88 toolchain for the Pokemon Mini. It's very much a quick port right now but testing and suggestions are welcome. Also, unless you need more than 2 colors right now or have some special needs, I would highly recommend using [logicplace's img2c](https://github.com/logicplace/pokemini-img2c) tool for the time being.

### Dependencies
#### Unix
GCC - 10+
#### Windows
MinGW - latest release usually through MSYS

DeVIL - https://github.com/DentonW/DevIL.git (for modern compiler support)

### Build from source
This is quite simple once you have DeVIL and its dependencies built (it builds fine on Linux and through WSL2 on Windows but had heavy dependency issues when I tried building through both Visual Studio and MinGW; the Windows build I released was a bit hacked together because of this).

* Run `g++ -o img2pm img2pm.cpp -lIL -lILU` and it should compile without issues as long as your DeVIL library files are in their standard locations for your OS. If they aren't or you would like to make the app portable, you'll have to build DeVIL as a static library, move the *.so/ *.dll files to wherever you cloned this repo and run `g++ -L. -o img2pm img2pm.cpp -lIL -lILU` to build it instead.
