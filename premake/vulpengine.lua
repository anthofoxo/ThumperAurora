project "vulpengine"
location "%{wks.location}/vendor/%{prj.name}"

files {
	"%{prj.location}/**.cpp",
	"%{prj.location}/**.hpp",
	"%{prj.location}/**.inl",
}

includedirs {
	"%{prj.location}/include/vulpengine",
	"%{wks.location}/vendor/glfw/include",
	"%{wks.location}/vendor/glad/include",
	"%{wks.location}/vendor/glm",
	"%{wks.location}/aurora/source",
}

defines {
	"GLFW_INCLUDE_NONE",
	"VP_FEATURE_RDOC_UNSUPPORTED"
}

filter { "configurations:dist", "system:windows" }
defines "VP_ENTRY_WINMAIN"