This is my WIP build of img2pm so that it can be used with the Epson C88 toolchain for the Pokemon Mini. It's very much a quick port right now but testing and suggestions are welcome. Also, unless you need more than 2 colors right now or have some special needs, I would highly recommend using [logicplace's img2c](https://github.com/logicplace/pokemini-img2c) tool for the time being.

### Dependencies
DeVIL - https://github.com/DentonW/DevIL.git (for modern compiler support; the package manager versions should technically work but are typically out of date and mostly unsupported by the dev)
cmake - 2.8+

#### Unix
GCC - 9+
#### Windows
MinGW (from MSYS2) - for creating the VS build files with CMake
Visual Studio 2015 Community+ - tested on VS 2022 for both 64-bit and 32-bit versions but VS2015 shouldn't have any issues

### Build from source
1. Download from git: `git clone https://github.com/bedwardly-down/img2pm.git`
2. Make a build directory in the img2pm directory
3. 
#### Unix
Run from inside the build directory: `cmake ../`
#### Windows
Run from a MSYS2 command prompt: `cmake -G "Visual Studio 17 2022" ../` (run `cmake -G` by itself to see how to do it for other VS versions)
4. 
#### Unix
Run `make`
#### Windows
Open the img2pm.sln file created with VS, right click the img2pm build target on the right side, click Properties, click VC++ directories, and add the path to where your DevIL lib and include directories are to their respective configurations here (is different per VS release and will require a bit of testing on your part). Run build and everything should build without issues.
#### Windows Only Note
If using the DevIL SDK, you may need to rename DevIL.lib to IL.lib to get this to compile properly but to get the outputted img2pm.exe file to run properly, you'll need to manually copy all of the dll files from the SDK to where img2pm.exe is at to get it to run properly.