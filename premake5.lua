include "Dependencies.lua"

workspace "EXT2FileSystem"
   architecture "x86"
   startproject "EXT2FS"
   configurations { "Debug", "Release" }

outputdir = "%{cfg.system}-%{cfg.architecture}-%{cfg.buildcfg}"

group "Dependencies"
   include "Dependencies/spdlog"
group ""

include "project"
