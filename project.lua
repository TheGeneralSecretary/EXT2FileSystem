project "EXT2FS"
kind "ConsoleApp"
language "C++"
cppdialect "C++17"
staticruntime "off"

targetdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")
objdir ("%{wks.location}/build/intermediates/" .. outputdir .. "/%{prj.name}")

pchheader "buildpch.h"
pchsource "Source/buildpch.cpp"

files
{
	"Source/**.h",
	"Source/**.cpp",
}

includedirs
{
	"Source",
	"%{IncludeDir.spdlog}",
}

links
{
	"spdlog",
}

filter "system:windows"
	systemversion "latest"

	defines
	{
		"BUILD_PLATFORM_WINDOWS",
	}

filter "system:linux"
	systemversion "latest"

	defines
	{
		"BUILD_PLATFORM_LINUX",
	}

filter "configurations:Debug"
	defines "BUILD_DEBUG"
	runtime "Debug"
	symbols "On"

filter "configurations:Release"
	defines "BUILD_RELEASE"
	runtime "Release"
	optimize "On"
