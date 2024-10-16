project "aurora"
debugdir "../working"
kind "ConsoleApp"

defines "GLFW_INCLUDE_NONE"

files {
    "%{prj.location}/**.cpp",
    "%{prj.location}/**.cc",
    "%{prj.location}/**.c",
    "%{prj.location}/**.hpp",
    "%{prj.location}/**.h"
}

includedirs {
    "%{prj.location}",
    "%{prj.location}/source",
    "%{prj.location}/vendor",
    "%{wks.location}/vendor/glfw/include",
    "%{wks.location}/vendor/imgui",
	"%{wks.location}/vendor/imgui/backends",
	"%{wks.location}/vendor/imgui/misc/cpp",
    "%{wks.location}/vendor/tinyfd",
    "%{wks.location}/vendor/lua/src",
    "%{wks.location}/vendor/glad/include",
    "%{wks.location}/vendor/glm",
    "%{wks.location}/vendor/assimp/include",
    "%{wks.location}/vendor/assimp_config",
    "%{wks.location}/vendor/vulpengine/include",
    "%{wks.location}/vendor/vulpengine/include/vulpengine",
}

links { "glfw", "imgui", "tinyfd", "lua", "glad", "assimp", "vulpengine" }

filter "system:windows"
files "%{prj.location}/*.rc"
links "opengl32"

filter { "configurations:release", "system:windows" }
kind "WindowedApp"
defines "HE_ENTRY_WINMAIN"

filter "system:linux"
links { "pthread", "dl" }