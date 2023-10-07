
# How to install dependencies

You need to install vulkan and sdl. I explained the steps for vcpkg but you use any other way to install and link these 2 libraries. Just make sure that vulkan-sdk is installed on your pc.

## using vcpkg

### To install vcpkg:
```
https://vcpkg.io/en/getting-started.html
```
### Dependencies

#### For windows:

To install vulkan:
```
vcpkg install vulkan:x64-windows
```
```For linux replace the windows with linux```

To install sdl2:
```
vcpkg install sdl2[vulkan] --triplet x64-windows
```
```For linux replace the windows with linux```

#### For linux:

To install vulkan:
```
vcpkg install vulkan:x64-windows
```
```For linux replace the windows with linux```

To install sdl2:
```
vcpkg install sdl2[vulkan] --triplet x64-windows
```

### Link vcpkg to your project

Add vcpkg to toochain and then run 
```
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
```

# How to build

Create a directory a build directory instead the project folder and change directory to that folder then execute following command:

## For make, ninja, etc

For release build:
```
cmake -DCMAKE_BUILD_TYPE=Release ..
```
For debug build:
```
cmake -DCMAKE_BUILD_TYPE=Debug ..
```
## For visual studio
```
cmake ..
```

You can use visual studio (recommended) or ninja on windows. For linux use the unix makefiles to run the project. 
