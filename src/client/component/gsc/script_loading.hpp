#pragma once
#include <xsk/gsc/engine/s1_pc.hpp>

namespace gsc
{
	extern std::unique_ptr<xsk::gsc::s1_pc::context> gsc_ctx;

	game::ScriptFile* find_script(game::XAssetType type, const char* name, int allow_create_default);
}
