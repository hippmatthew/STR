workspace "STR"
  configurations { "default" }
  platforms { "MacOS-ARM64", "Linux-x86_64" }
  location "."

  filter { "platforms:MacOS-ARM64" }
    system "macosx"
    architecture "ARM64"

  filter { "platforms:Linux-x86_64" }
    system "linux"
    architecture "x86_64"

  project "STR"
    location "src"
    filename "STR"

    kind "ConsoleApp"
    language "C++"
    cppdialect "c++20"

    targetdir "bin"
    objdir "obj"
    targetname "str"
    
    files {
      "src/include/*.hpp",
      "src/*.cpp",
      "shaders/*.vert",
      "shaders/*.frag"
    }

    links {
      "vulkan",
      "glfw",
      "vecs"
    }

    includedirs { "." }

    filter { "files:shaders/*.vert"}
      buildmessage "compiling vertex shader %{file.relpath}"
      buildcommands { "glslc -o ../spv/%{file.basename}.vert.spv %{file.relpath}" }
      buildoutputs { "../spv/%{file.basename}.vert.spv" }

    filter { "files:shaders/*.frag" }
      buildmessage "compiling fragment shader %{file.relpath}"
      buildcommands { "glslc -o ../spv/%{file.basename}.frag.spv %{file.relpath}" }
      buildoutputs { "../spv/%{file.basename}.frag.spv" }
    
    filter { "platforms:MacOS-ARM64" }
      includedirs {
        "/opt/homebrew/include",
        "/usr/local/include"
      }

      libdirs {
        "/opt/homebrew/lib",
        "/usr/local/lib"
      }

      linkoptions { "-rpath /usr/local/lib" }