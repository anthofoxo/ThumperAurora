# Thumper Aurora

Aurora is a binary data viewer and editor for [Thumper](https://thumpergame.com/).

## Cloning
`git clone https://github.com/anthofoxo/ThumperAurora -j8 --recurse-submodules`

## Building
[Premake5](https://premake.github.io/) is used as the build system. Have this installed.
If you're too lazy to properly set this up, just toss the `premake5` executable in your system32 folder.

With premake installed. Run `premake5 vs2022` or `premake5 gmake2` to generate the project files.

With these project files. The application should build without any issue.

## Dependencies
* https://github.com/native-toolkit/libtinyfiledialogs/tree/2.9.3
* https://github.com/glfw/glfw/tree/3.4
* https://github.com/ocornut/imgui/tree/v1.90.9-docking
* https://github.com/anthofoxo/lua/tree/5.4.6