# Using vku

This page explains about how to build and install *vku* for your system, and how to seamlessly integrate it into your project.

## 1. Usage Options

*vku* uses CMake as the default build tool. With its versatility, you can choose the way you want to use *vku*. Options are:

- Manually clone, build and install *vku* into your system. After this, you can use *vku* via `find_package(vku CONFIG REQUIRED)` in your future CMake project.
- Using vcpkg: *vku* is initially intended to be used with [vcpkg](https://github.com/microsoft/vcpkg). For this case, you don't have to install it manually, but may have to set some triplet and chainload settings for your vcpkg project.
- Using CMake's `FetchContent` or [`CPM.cmake`](https://github.com/cpm-cmake/CPM.cmake): with CMake's built-in feature, you can easily include *vku* in your project without any additional setup.

## 2. Prerequisites

### 2.1. Dependencies

*vku* depends on:

- [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp): C++ bindings for Vulkan API
- [VulkanMemoryAllocator-Hpp](https://github.com/YaaZ/VulkanMemoryAllocator-Hpp): C++ bindings for Vulkan Memory Allocator

If you're using vcpkg, these dependencies are automatically installed. Otherwise, you must install them manually, then following CMake commands would available.

```CMake
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(VulkanMemoryAllocator-Hpp CONFIG REQUIRED)
```

If you're using [shaderc](https://github.com/google/shaderc) runtime GLSL compilation feature (can be enabled by setting `VKU_USE_SHADERC` to `ON`), the installed Vulkan SDK must have `shaderc_combined` components and following CMake commands must be available.

```CMake
find_package(Vulkan COMPONENTS shaderc_combined REQUIRED)
```

### 2.2. Compilers and Build Tools

*vku* uses the cutting edge C++20 feature, [module](https://en.cppreference.com/w/cpp/language/modules), which is not yet widely supported by most compilers. Unless you're using Windows and up-to-date MSVC compiler, most of the case you have to manually specify the non-system default compiler.

Followings are the **minimum** requirement for building *vku*.

- Compiler
  - Clang 18.1.2 or later
  - MSVC 19.40 or later
- Build tool
  - CMake 3.28 or later
  - Ninja 1.11

If you want to build *vku* with [C++23 standard library module](https://en.cppreference.com/w/cpp/standard_library#Importing_modules), you need:

- Build tool
  - CMake 3.30 or later (for experimental `import std;` support: see [here](https://www.kitware.com/import-std-in-cmake-3-30/) for the detail)

## 3. Building and Installing vku

> [!TIP]
> *vku* uses GitHub Actions for CI test. If you're struggling for building and installing *vku*, you can refer to the CI script ([Linux](../.github/workflows/linux.yml), [Windows](../.github/workflows/windows.yml) for the detail.)

Clone the repository and make your shell in the repository root directory.

```sh
git clone https://github.com/stripe2933/vku.git
cd vku
```

If you're using vcpkg, make sure `VCPKG_ROOT` environment variable is set to your vcpkg root directory.

### 3.1. Windows + MSVC

*vku* provides you `CMakePresets.json` file for this configuration. Use `vcpkg` preset if you're managing dependencies with vcpkg, otherwise use `default` preset.

```sh
cmake --preset=vcpkg # Configure
cmake --build build --target install # Build and install
```

After the installation, package config files will be located in `build/install` directory. You can pass your own `CMAKE_INSTALL_PREFIX` when configuration time to change the installation directory (e.g. `C:\Program Files\vku`).

In future CMake project, you can use `find_package(vku CONFIG REQUIRED)` to use *vku*.

### <a name="macos-clang"></a>3.2. macOS + Clang

For macOS, you'll use the combination of Clang + libc++.

> [!IMPORTANT]
> You must use the LLVM Clang, not the Apple Clang. The Apple Clang does not support the C++20 module feature.

You can pass these CMake configuration parameters via either CLI or your own `CMakeUserPresets.json`.

- Passing CLI parameters:
    ```sh
    cmake --preset=vcpkg \
        -DCMAKE_C_COMPILER="/opt/homebrew/opt/llvm/bin/clang-18" \
        -DCMAKE_CXX_COMPILER="/opt/homebrew/opt/llvm/bin/clang++" \
        -DCMAKE_CXX_FLAGS="-nostdinc++ -nostdlib++ -isystem /opt/homebrew/opt/llvm/include/c++/v1" \
        -DCMAKE_EXE_LINKER_FLAGS="-L /opt/homebrew/opt/llvm/lib/c++ -Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++ -lc++" # Configure
    cmake --build build --target install # Build and install
    ```
- Use CMake presets:

  `CMakeUserPresets.json`
  ```json
  {
    "version": 6,
    "configurePresets": [
      {
        "name": "clang",
        "displayName": "Clang",
        "inherits": "vcpkg",
        "cacheVariables": {
          "CMAKE_C_COMPILER": "/opt/homebrew/opt/llvm/bin/clang-18",
          "CMAKE_CXX_COMPILER": "/opt/homebrew/opt/llvm/bin/clang++",
          "CMAKE_CXX_FLAGS": "-nostdinc++ -nostdlib++ -isystem /opt/homebrew/opt/llvm/include/c++/v1",
          "CMAKE_EXE_LINKER_FLAGS": "-L /opt/homebrew/opt/llvm/lib/c++ -Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++ -lc++"
        }
      }
    ]
  }
  ```
  After file creation, you can use `--preset=clang` to configure.

> [!NOTE]
> About `CMAKE_CXX_FLAGS` and `CMAKE_EXE_LINKER_FLAGS` variable, see [libc++ documentation](https://releases.llvm.org/18.1.4/projects/libcxx/docs/UsingLibcxx.html#id4). These options make your compiler use the homebrew provided libc++, instead of the system default.

After the installation, package config files will be located in `build/install` directory. You can pass your own `CMAKE_INSTALL_PREFIX` when configuration time to change the installation directory (e.g. `/usr/local`).

In future CMake project, you can use `find_package(vku CONFIG REQUIRED)` to use *vku*.

### 3.3. Linux + Clang

For Linux, you'll use Clang and you can choose either libc++ or libstdc++ for STL.

#### Using libstdc++

*vku* provides you `CMakePresets.json` file for this configuration. Use `vcpkg` preset if you're managing dependencies with vcpkg, otherwise use `default` preset.

```sh
cmake --preset=vcpkg \
  -DCMAKE_C_COMPILER=/usr/bin/clang-18 \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++-18 # Configure
cmake --build build --target install # Build and install
```

#### Using libc++

You have to specify `-stdlib=libc++` flag to your compiler.

```sh
cmake --preset=vcpkg \
  -DCMAKE_C_COMPILER=/usr/bin/clang-18 \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++-18 \
  -DCMAKE_CXX_FLAGS="-stdlib=libc++" \
  -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++ -lc++abi" # Configure
cmake --build build --target install # Build and install
```

Or you can make your own `CMakePresets.json` script (like [macOS + Clang](#macos-clang)).

After the installation, package config files will be located in `build/install` directory. You can pass your own `CMAKE_INSTALL_PREFIX` when configuration time to change the installation directory (e.g. `/usr/local`).

In future CMake project, you can use `find_package(vku CONFIG REQUIRED)` to use *vku*.

## 4. Directly use *vku* in Your CMake Project

Most of the compiler specific setup would be same as the above. You have to set the proper compilers for *vku*'s requirement.

If you're using vcpkg, since *vku* does not officially in the vcpkg ports, you have to set some ports overlay and triplet settings.

> [!NOTE]
> I'm currently working for make *vku* to be available with vcpkg official ports.

### 4.1. Using vcpkg

#### 4.1.1. Add vku to your overlay ports

Download [this overlay port declaration](https://github.com/stripe2933/vk-deferred/tree/main/overlays/vku) into `overlays` folder in your project.

Then, pass `VCPKG_OVERLAY_PORTS=${sourceDir}/overlays` to your vcpkg command. For example:

`CMakePresets.json`
```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "vcpkg",
      "displayName": "vcpkg-based dependency management",
      "inherits": "default",
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
      "cacheVariables": {
        "VCPKG_OVERLAY_PORTS": "${sourceDir}/overlays"
      }
    }
  ]
}
```

#### 4.1.2. Set the Proper Triplet and Chainload Toolchain

If your system default compiler does not support C++20 module, this step is mandatory. Here's the example for x64 Linux + libc++.

`triplets/x64-linux-clang.cmake`
```cmake
set(VCPKG_TARGET_ARCHITECTURE x64) # your target architecture
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux) # your target system name
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/../clang-toolchain.cmake)
```

`clang-toolchain.cmake`
```cmake
set(CMAKE_C_COMPILER /usr/bin/clang-18) # your clang path
set(CMAKE_CXX_COMPILER /usr/bin/clang++-18) # your clang++ path
set(CMAKE_CXX_FLAGS "-stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++ -lc++abi")
```

After setting custom triplets and toolchain file, make `VCPKG_OVERLAY_TRIPLETS` and `VCPKG_TARGET_TRIPLET` point to your triplet file and target triplet.

`CMakePresets.json`
```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "vcpkg",
      "displayName": "vcpkg-based dependency management",
      "inherits": "default",
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
      "cacheVariables": {
        "VCPKG_OVERLAY_PORTS": "${sourceDir}/overlays",
        "VCPKG_OVERLAY_TRIPLETS": "${sourceDir}/triplets",
        "VCPKG_TARGET_TRIPLET": "x64-linux-clang"
      }
    }
  ]
}
```

All done! Now configuring your project with `--preset=vcpkg` would automatically build *vku*, and you can use it as `find_package(vku CONFIG REQUIRED)`.

### 4.2. Using CMake's `FetchContent` or `CPM.cmake`

1. Using `FetchContent`:
    ```cmake
    FetchContent_Declare(
        vku
        URL https://github.com/stripe2933/vku/archive/refs/tags/v0.1.0.tar.gz
    )
    FetchContent_MakeAvailable(vku)
    ```
   
2. Using `CPM.cmake`:
    ```cmake
    CPMAddPackage("gh:stripe2933/vku@0.1.0")
    ```
   
After this, you can use `find_package(vku CONFIG REQUIRED)` in your project.

## 5. Configuration Options

Here are some CMake configuration options. You can pass these options via either CLI, CMake presets or vcpkg feature.

| Option                           | Description                                                  | Default |     vcpkg feature      |
|----------------------------------|--------------------------------------------------------------|---------|:----------------------:|
| `VKU_USE_STD_MODULE`             | Use the standard library module for compilation.             | `OFF`   |      `std-module`      |
| `VKU_USE_SHADERC`                | Add runtime GLSL compilation feature by shaderc.             | `OFF`   |       `shaderc`        |
| `VKU_DEFAULT_DYNAMIC_DISPATCHER` | Use the vk::DispatchLoaderDynamic as the default dispatcher. | `OFF`   |  `dynamic-dispatcher`  |
| `VKU_ENABLE_TEST`                | Enable the test targets.                                     | `OFF`   |           -            |