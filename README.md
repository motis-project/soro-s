<p align="center"><img src="logo.png" width="350"></p>

![Linux & Mac OS Build](https://github.com/motis-project/rapid/workflows/Unix%20Build/badge.svg)
![Windows Build](https://github.com/motis-project/rapid/workflows/Windows%20Build/badge.svg)

## SORO-S Setup (Ubuntu 20.04)

If this setup does not work for you check out the unix.yml or windows.yml file
for the github actions. There should
always be a configuration for a successful build.

Newer versions of Ubuntu or other distributions might not need to excute all
steps, since some packages might already be
included.

### Tools:

- Git
- CMake >= 3.19
- GCC >= 11
- Clang >= 14
- Ninja

### Steps:

Install either clang or GCC or both.

#### Installing Clang

Add Repositories and signing key.

```shell
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
echo "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-15 main" | sudo tee -a /etc/apt/sources.list
sudo apt update
```

Install clang and additional tooling.

```shell
sudo apt install clang-15 lldb-15 lld-15 clangd-15 clang-tidy-15 clang-format-15 clang-tools-15 llvm-15-dev llvm-15-tools libomp-15-dev libc++-15-dev libc++abi-15-dev libclang-common-15-dev libclang-15-dev libclang-cpp15-dev libunwind-15-dev
```

#### Installing GCC

Add Toolchains Repo.

```shell
sudo add-apt-repository ppa:ubuntu-toolchain-r/test && sudo apt update
```

Install GCC.

```shell
sudo apt install g++-11
```

#### Installing remaining tools

First add the kitware repository for the latest cmake version. To do this add
the kitware signing key.

```shell
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
```

Then add the kitware repository itself.

```shell
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
```

Then install the remaining tools.

```shell
sudo apt install git cmake ninja-build
```

#### Cloning the SORO-S repository

Clone the repository.

```shell
git clone git@github.com:motis-project/soro-s.git
```

#### Building the tests

Invoke cmake with the 'clang-release' preset for a build with clang ..

```shell
cd soro-s && cmake --preset clang-release
```

.. or 'gcc-release' for a build with gcc.

```shell
cd soro-s && cmake --preset gcc-release
```

Navigate to the build directory and build the tests for a clang build ..

```shell
cd build/clang-release && ninja
```

.. or a gcc build.

```shell
cd build/gcc-release && ninja
```

Test & Run.

```shell
./soro-test
```

### Webinterface

To use the webinterface locally you need to do the following:

#### Building the web interface

Build the webinterface by invoking the following in a build directory.

```shell
ninja soro-server
```

Start the soro-server.

```shell
./soro-server --resource_dir ../../resources
```

You can access the interface now on [localhost:8080](http://localhost:8080).

### SORO-S CMake Flags

These are the custom CMake flags. Turn them on by passing -D{Flag Name}=On to
CMake.

| Flag Name | Effect |
|-----------|--------|
|SORO_LINT  | Runs clang-tidy alongside the compilation process.|
|SORO_SAN | Only works when clang is set as the compiler. Compiles SORO-S with the C/CXX flags "-fsanitize=address,undefined".|
|SERIALIZE | When enabled uses  [cista](https://github.com/felixguendling/cista) to enable serialization.|
|USE_CISTA_RAW | Only works when SERIALIZE=On. Uses the cista::raw instead of cista::offset. |
|SORO_CUDA | Enables GPU-accelerated computing for select tasks. |

## SORO-S Setup (Visual Studio 2022)

Please make sure that you have the following tools installed and available in
the build environment.

- Git
- Ninja
- CMake
- MSVC >= 14.34

After adding the repository in Visual Studio select either the MSVC Debug or
MSVC Release preset to compile.

## Not supported:

- MinGW
