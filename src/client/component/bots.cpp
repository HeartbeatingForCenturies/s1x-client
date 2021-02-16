#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "command.hpp"
#include "scheduler.hpp"
#include "game/game.hpp"
#include "party.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace bots
{
	namespace
	{
		bool can_spawn()
		{
			if (party::get_client_count() < *game::mp::svs_numclients)
			{
				return true;
			}
			return false;
		}

		void add_bot()
		{
			if (!can_spawn())
			{
				return;
			}

			// SV_BotGetRandomName
			auto* bot_name = reinterpret_cast<const char* (*)()>(0x1404267E0)();
			auto* bot_ent = game::SV_AddBot(bot_name);
			if (bot_ent)
			{
				game::SV_SpawnTestClient(bot_ent);
			}
			else if (can_spawn()) // workaround since first bot won't ever spawn
			{
				add_bot();
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (game::environment::is_sp())
			{
				return;
			}

			command::add("spawnBot", [](const command::params& params)
			{
				if (!game::SV_Loaded()) return;

				auto num_bots = 1;
				if (params.size() == 2)
				{
					num_bots = atoi(params.get(1));
				}

				for (auto i = 0; i < (num_bots > *game::mp::svs_numclients ? *game::mp::svs_numclients : num_bots); i++)
				{
					scheduler::once(add_bot, scheduler::pipeline::server, 100ms * i);
				}
			});
		}
	};
}

//REGISTER_COMPONENT(bots::component)
