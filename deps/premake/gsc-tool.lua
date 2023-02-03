gsc_tool = {
	source = path.join(dependencies.basePath, "gsc-tool/src"),
}

function gsc_tool.import()
	links { "xsk-gsc-s1", "xsk-gsc-utils" }
	gsc_tool.includes()
end

function gsc_tool.includes()
	includedirs {
		path.join(gsc_tool.source, "s1"),
		path.join(gsc_tool.source, "utils"),
		path.join(gsc_tool.source, "gsc"),
		gsc_tool.source,

		path.join(dependencies.basePath, "extra/gsc-tool"),
	}
end

function gsc_tool.project()
	project "xsk-gsc-utils"
		kind "StaticLib"
		language "C++"

		files {
			path.join(gsc_tool.source, "utils/**.hpp"),
			path.join(gsc_tool.source, "utils/**.cpp"),
		}

		includedirs {
			path.join(gsc_tool.source, "utils"),
			gsc_tool.source,
		}

		zlib.includes()
		fmt.includes()

	project "xsk-gsc-s1"
		kind "StaticLib"
		language "C++"

		filter "action:vs*"
			buildoptions "/Zc:__cplusplus"
		filter {}

		files {
			path.join(gsc_tool.source, "s1/s1_pc.hpp"),
			path.join(gsc_tool.source, "s1/s1_pc.cpp"),
			path.join(gsc_tool.source, "s1/s1_pc_code.cpp"),
			path.join(gsc_tool.source, "s1/s1_pc_func.cpp"),
			path.join(gsc_tool.source, "s1/s1_pc_meth.cpp"),
			path.join(gsc_tool.source, "s1/s1_pc_token.cpp"),

			path.join(gsc_tool.source, "gsc/misc/*.hpp"),
			path.join(gsc_tool.source, "gsc/misc/*.cpp"),
			path.join(gsc_tool.source, "gsc/*.hpp"),
			path.join(gsc_tool.source, "gsc/*.cpp"),

			path.join(dependencies.basePath, "extra/gsc-tool/gsc_interface.cpp"),
		}

		includedirs {
			path.join(gsc_tool.source, "s1"),
			gsc_tool.source,
			path.join(dependencies.basePath, "extra/gsc-tool"),
		}

		fmt.includes()
end

table.insert(dependencies, gsc_tool)
