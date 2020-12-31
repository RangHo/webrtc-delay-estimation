# webrtc-delay-estimation
Delay estimation logic extracted from WebRTC.

## How to build

This repository uses `CMake` to automate cross-platform builds. If it fails to build, please [create an issue](https://github.com/RangHo/webrtc-delay-estimation/issues/new) with the following informations:
- CMake version
- Platform (operating system with CPU architecture)
- Compiler name and version
- Error message produced by CMake

### On Windows

1. Install **Visual Studio** and **CMake**. For Visual Studio 2017 and newer, instead of installing CMake by yourself, you can install a Visual Studio component called `Visual C++ Tools for CMake`, which greatly simplifies the process. If you are using an older version of Visual Studio, install standalone CMake.

![Screenshot of Visual Studio Installer installing CMake](https://docs.microsoft.com/ko-kr/cpp/build/media/cmake-install.png)

2. Clone this repository with submodules.

```sh
git clone --recurse-submodules https://github.com/RangHo/webrtc-delay-estimation
```

#### VS2017 or newer

3. You can open the project by clicking **File > Open > Folder** in Visual Studio, and selecting the `webrtc-delay-estimation` folder you just cloned. Visual Studio should recognize `CMakeLists.txt` automatically. You can build the project as if it is a regular Visual Studio project.

#### VS2015 or older

3. For older versions of Visual Studio, you need to use CMake command line to generate project files. Generate `.vcxproj` files using the following command. Replace `<generator>` with the name of [Visual Studio generator](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#visual-studio-generators) matching the version of Visual Studio you are using.

```sh
mkdir build
cd build
cmake -G "<generator>" ..
```

4. Open the `.sln` file CMake created, and build the solution.

### On macOS and Linux (and any other POSIX systems)

1. Install **CMake**. For most distributions, there should be a package for your default package manager.

```sh
# Ubuntu
sudo apt-get install cmake

# Arch Linux
sudo pacman -S cmake

# Homebrew
brew install cmake
```

2. Clone the repository and `cd` into it.

```sh
git clone --recurse-submodules https://github.com/RangHo/webrtc-delay-estimation
cd webrtc-delay-estimation
```

3. Create a build directory and build.

```sh
mkdir build && cd build
cmake ..
cmake --build . --target webrtc-delay-estimation
```
