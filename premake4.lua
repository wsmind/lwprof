local buildRoot = "tmpbuildroot"
if _ACTION then
	buildRoot = "build/" .. os.get() .. "-" .. _ACTION
end

solution "lwprof"
	location(buildRoot)
	configurations {"debug", "release"}
	kind "ConsoleApp"
	language "C++"
	objdir(buildRoot .. "/obj")

configuration "debug"
	flags "Symbols"
	targetdir(buildRoot .. "/bin/debug")

configuration "release"
	flags "Optimize"
	targetdir(buildRoot .. "/bin/release")

project "fileio"
	files {"src/*.cpp", "src/*.hpp", "test/fileio.cpp"}

project "multithread"
	files {"src/*.cpp", "src/*.hpp", "test/multithread.cpp"}
	if os.is("linux") then
		links {"pthread"}
	end
