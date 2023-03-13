-- premake.lua
workspace "dsa-project"
  configurations {"debug", "release"}
  architecture "x86_64"

  filter "configurations:debug"
    symbols "On"
    defines "DEBUG"

  filter "configurations:release"
    optimize "On"
    defines "NDEBUG"

  filter {}

project "mgs"
  kind "ConsoleApp"
  language "C++"
  cppdialect "C++17"

  location "%{wks.location}/build"
  targetdir "%{prj.location}/%{cfg.buildcfg}"
  objdir "%{prj.location}/%{cfg.buildcfg}/obj"

  files { "%{wks.location}/source/**.cpp", "%{wks.location}/source/**.hpp" }
  includedirs { "%{wks.location}/source" }

  files {
    "./extern/imgui/imgui.cpp",
    "./extern/imgui/imgui_demo.cpp",
    "./extern/imgui/imgui_draw.cpp",
    "./extern/imgui/imgui_widgets.cpp",
    "./extern/imgui/imgui_tables.cpp",

    "./extern/imgui_impl_sdl2/imgui_impl_sdl2.cpp",
    "./extern/imgui_impl_sdl2/imgui_impl_sdlrenderer.cpp"
  }
  includedirs { "./extern/imgui", "./extern/imgui_impl_sdl2" }

  filter "not system:windows"
    links { "SDL2", "SDL2main" }

  filter "system:windows"
    includedirs { "./extern/SDL2-2.26.1/" }
    libdirs { "./extern/SDL2-2.26.1/lib/x64" }
    links { "SDL2.lib", "SDL2main.lib" }
    prebuildcommands { "pushd ..\\extern && .\\get_sdl2.bat && popd" }
    postbuildcommands { "copy ..\\extern\\SDL2-2.26.1\\lib\\x64\\SDL2.dll .\\%{cfg.buildcfg}\\" }

  filter {}
