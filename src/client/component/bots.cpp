#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"
#include "scheduler.hpp"
#include "party.hpp"
#include "network.hpp"
#include "server_list.hpp"

#include "game/game.hpp"
#include "game/scripting/execution.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/cryptography.hpp>

namespace bots
{
	namespace
	{
		bool can_add()
		{
			if (party::get_client_count() < *game::mp::svs_numclients)
			{
				return true;
			}
			return false;
		}

		void bot_team_join(const unsigned int entity_num)
		{
			const game::scr_entref_t entref{static_cast<uint16_t>(entity_num), 0};
			scheduler::once([entref]()
			{
				scripting::notify(entref, "luinotifyserver", {"team_select", 2});
				scheduler::once([entref]()
				{
					auto* _class = utils::string::va("class%d", utils::cryptography::random::get_integer() % 5);
					scripting::notify(entref, "luinotifyserver", {"class_select", _class});
				}, scheduler::pipeline::server, 2s);
			}, scheduler::pipeline::server, 2s);
		}

		void spawn_bot(const int entity_num)
		{
			game::SV_SpawnTestClient(&game::mp::g_entities[entity_num]);
			if (game::Com_GetCurrentCoDPlayMode() == game::CODPLAYMODE_CORE)
			{
				bot_team_join(entity_num);
			}
		}

		void add_bot()
		{
			if (!can_add())
			{
				return;
			}

			// SV_BotGetRandomName
			const auto* const bot_name = game::SV_BotGetRandomName();
			auto* bot_ent = game::SV_AddBot(bot_name);
			if (bot_ent)
			{
				spawn_bot(bot_ent->s.entityNum);
			}
			else if (can_add()) // workaround since first bot won't ever spawn
			{
				add_bot();
			}
		}

		utils::hook::detour get_bot_name_hook;
		volatile bool bot_names_received = false;
		std::vector<std::string> bot_names{};

		const char* get_random_bot_name()
		{
			if (!bot_names_received || bot_names.empty())
			{
				return get_bot_name_hook.invoke<const char*>();
			}

			const auto index = utils::cryptography::random::get_integer() % bot_names.size();
			const auto& name = bot_names.at(index);

			return utils::string::va("%.*s", static_cast<int>(name.size()), name.data());
		}

		void update_bot_names()
		{
			bot_names_received = false;

			game::netadr_s master{};
			if (server_list::get_master_server(master))
			{
				printf("Getting bots...\n");
				network::send(master, "getbots");
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

			get_bot_name_hook.create(game::SV_BotGetRandomName, get_random_bot_name);

			command::add("spawnBot", [](const command::params& params)
			{
				if (!game::SV_Loaded() || game::VirtualLobby_Loaded()) return;

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

			scheduler::on_game_initialized([]()
			{
				update_bot_names();
				scheduler::loop(update_bot_names, scheduler::main, 1h);
			}, scheduler::main);

			network::on("getbotsResponse", [](const game::netadr_s& target, const std::string_view& data)
			{
				game::netadr_s master{};
				if (server_list::get_master_server(master) && !bot_names_received && target == master)
				{
					bot_names = utils::string::split(std::string(data), '\n');
					bot_names_received = true;
				}
			});
		}
	};
}

REGISTER_COMPONENT(bots::component)
