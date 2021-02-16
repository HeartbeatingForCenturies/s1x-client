#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "videos.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace videos
{
	class video_replace
	{
	public:
		std::string replace;
		std::string with;
	};

	namespace
	{
		template<typename T>
		T* find_vid(std::vector<T>* vec, const std::string& name)
		{
			for (auto i = 0ull; i < vec->size(); i++)
			{
				if (name == vec->at(i).replace)
				{
					return &vec->at(i);
				}
			}
			return nullptr;
		}
	}

	static std::vector<video_replace> replaces;

	void replace(const std::string& what, const std::string& with)
	{
		video_replace vid;
		vid.replace = what;
		vid.with = with;
		replaces.push_back(std::move(vid));
	}

	namespace
	{
		utils::hook::detour playvid_hook;

		void playvid(const char* name, int a2, int a3)
		{
			auto* vid = find_vid(&replaces, name);
			if (vid)
			{
				char path[256];
				game::Sys_BuildAbsPath(path, 256, game::SF_VIDEO, vid->with.data(), ".bik");

				if (game::Sys_FileExists(path))
				{
					name = vid->with.data();
				}
			}

			return playvid_hook.invoke<void>(name, a2, a3);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			playvid_hook.create(SELECT_VALUE(0x1404591C0, 0x140575AA0), &playvid);

			if (game::environment::is_mp())
			{
				replace("menus_bg_comp2", "menus_bg_s1x");
				replace("mp_menus_bg_options", "menus_bg_s1x_blur");
			}
			else if (game::environment::is_sp())
			{
				replace("sp_menus_bg_main_menu", "menus_bg_s1x_sp");
				replace("sp_menus_bg_campaign", "menus_bg_s1x_sp");
				replace("sp_menus_bg_options", "menus_bg_s1x_sp");
			}
		}
	};
}

//REGISTER_COMPONENT(videos::component)