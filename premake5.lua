workspace "aurora"
architecture "x86_64"
configurations { "debug", "release" }
startproject "aurora"

flags "MultiProcessorCompile"
language "C++"
cppdialect "C++latest"
cdialect "C17"
staticruntime "On"
stringpooling "On"
editandcontinue "Off"

kind "StaticLib"
targetdir "%{wks.location}/bin/%{cfg.system}_%{cfg.buildcfg}"
objdir "%{wks.location}/bin_obj/%{cfg.system}_%{cfg.buildcfg}"

filter "configurations:debug"
runtime "Debug"
optimize "Debug"
defines "VP_DEBUG"
symbols "On"

filter "configurations:release"
runtime "Release"
optimize "Speed"
symbols "Off"
defines "VP_RELEASE"
flags { "LinkTimeOptimization", "NoBufferSecurityCheck" }

filter "system:windows"
systemversion "latest"
defines { "NOMINMAX", "VP_WINDOWS" }
buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus", "/experimental:c11atomics" }

filter "system:linux"
defines "VP_LINUX"

filter { "system:linux", "language:C++" }
buildoptions "-std=c++23"

filter {}

group "dependencies"
for _, matchedfile in ipairs(os.matchfiles("premake/*.lua")) do
    include(matchedfile)
end
group ""

include "aurora/build.lua"