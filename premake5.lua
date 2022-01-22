workspace "BSPViewer"
	architecture "x86_64"
	flags {
		"MultiProcessorCompile"
	}
	configurations {
		"Debug",
		"Release"
	}
	debugdir "data"
	targetdir (path.join("bin", "%{cfg.buildcfg}"))
	objdir (path.join("bin", "obj", "%{cfg.buildcfg}"))
	
project "BSPViewer"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++11"
	includedirs {
		"code",
		path.join("extern", "SDL2-2.0.18", "include"),
		path.join("extern", "glm-0.9.9.8"),
		path.join("extern", "glad", "include")
	}
	libdirs {
		path.join("extern", "SDL2-2.0.18", "lib", "x64")
	}
	files {
		"code/**.h",
		"code/main.cpp",
		"extern/glad/src/glad.c"
	}
	links {
		"SDL2",
		"SDL2main"
	}
	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "Full"