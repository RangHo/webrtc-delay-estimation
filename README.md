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

## How to use

This repository provides 3 CMake targets as follows:
- `webrtc-delay-estimation`: A static library exposing delay estimation function as declared in [`include/webrtc-delay-estimation.h`](https://github.com/RangHo/webrtc-delay-estimation/blob/main/include/webrtc_delay_estimation.h)
- `delay-estimator`: An executable that uses the library above to estimate delay in between two WAV files
- `webrtc-delay-estimation-tests`: A testing executable that generates random samples and tries to estimate delay using them

### `webrtc-delay-estimation` library

This library exposes the extracted WebRTC components and an additional helper function. By default, linking this library using CMake's `target_link_libraries` directive includes definition for the helper function only. To access raw WebRTC components, copy the relevant `.h` files from `src/` directory.

Example `CMakeLists.txt` file would look like:

```cmake
add_executable (my-executable
  "main.cc"
  # ...
)

target_link_libraries (my-executable PRIVATE
  webrtc-delay-estimation
)
```

### `delay-estimator` binary

This command line utility uses `webrtc-delay-estimation` library above to find delay from two different WAV files. Unless given a `-v` switch, the program will write the estimated delay in samples to `stdout`. It takes two positional arguments: `render` and `capture`. The former is the WAV file of the far end, and the latter is the WAV file captured by the local microphone.

#### Usage

`delay-estimator [-hv] [-f integer] [-d {2,4,8}] /path/to/render /path/to/capture`

#### Argument information

- (required, positional) `/path/to/render`: path to the render WAV file.
- (required, positional) `/path/to/capture`: path to the capture WAV file.
- (optional) `-h` or `--help`: displays help message.
- (optional) `-v` or `--verbose`: show additional information when executing the program.
- (optional) `-f integer` or `--filter integer`: Use `integer` number of filters when estimating delay. (default: 10)
- (optional) `-d {2,4,8}` or `--downsampling-factor {2,4,8}`: sets the down sampling factor. The factor can be either 2, 4, or 8. (default: 8)

### `webrtc-delay-estimation-tests` binary

This target builds a testing binary using [Catch2](https://github.com/catchorg/Catch2) testing framework. Build and run the binary to test the functionality of the library.

```sh
# You can run the binary directly
cd build/tests
./webrtc-delay-estimation-tests

# ... or you can just use ctest to automate things
ctest
```
