#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace images
{
	namespace
	{
		utils::hook::detour load_texture_hook;

		static_assert(sizeof(game::GfxImage) == 104);
		static_assert(offsetof(game::GfxImage, name) == (sizeof(game::GfxImage) - sizeof(void*)));
		static_assert(offsetof(game::GfxImage, texture) == 56);
	
		void load_texture_stub(game::GfxImageLoadDef** load_def, game::GfxImage* image)
		{
			load_texture_hook.invoke(load_def, image);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (!game::environment::is_mp()) return;

			load_texture_hook.create(0x1405A21F0, load_texture_stub);
		}
	};
}

REGISTER_COMPONENT(images::component)
