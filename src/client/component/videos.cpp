#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "videos.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

namespace videos
{
	class video_replace
	{
	public:
		const char* replace;
		const char* with;
	};

	namespace
	{
		template<typename T>
		T* find_vid(std::vector<T>* vec, const char* name)
		{
			for (auto i = 0; i < vec->size(); i++)
			{
				if (!strcmp(name, vec->at(i).replace))
				{
					return &vec->at(i);
				}
			}
			return nullptr;
		}
	}

	static std::vector<video_replace> replaces;

	void replace(const char* what, const char* with)
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
			if (vid && utils::io::file_exists(utils::string::va("raw\\video\\%s.bik", vid->with)))
			{
				name = vid->with;
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
				replace("menus_bg_comp2", "menus_bg_s1-mod");
				replace("mp_menus_bg_options", "menus_bg_s1-mod_blur");
			}
		}
	};
}

REGISTER_COMPONENT(videos::component)