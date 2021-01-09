#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "party.hpp"

#include "dvars.hpp"
#include "command.hpp"
#include "network.hpp"
#include "scheduler.hpp"
//#include "server_list.hpp"

#include "steam/steam.hpp"

#include <utils/string.hpp>
#include <utils/info_string.hpp>
#include <utils/cryptography.hpp>
#include <utils/hook.hpp>

namespace party
{
	namespace
	{
		struct
		{
			game::netadr_s host{};
			std::string challenge{};
		} connect_state;

		std::string sv_motd;
		 
		void perform_game_initialization()
		{
			// This fixes several crashes and impure client stuff
			command::execute("onlinegame 1", true);
			command::execute("exec default_xboxlive.cfg", true);
			command::execute("xstartprivateparty", true);
			command::execute("xblive_privatematch 1", true);
			command::execute("startentitlements", true);
		}

		void connect_to_party(const game::netadr_s& target, const std::string& mapname, const std::string& gametype)
		{
			if (game::environment::is_sp())
			{
				return;
			}

			if (game::Live_SyncOnlineDataFlags(0))
			{
				scheduler::once([=]()
				{
					connect_to_party(target, mapname, gametype);
				}, scheduler::pipeline::main, 1s);
				return;
			}

			perform_game_initialization();

			// CL_ConnectFromParty
			char session_info[0x100] = {};
			reinterpret_cast<void(*)(int, char*, const game::netadr_s*, const char*, const char*)>(0x140209360)(
				0, session_info, &target, mapname.data(), gametype.data());
		}

		std::string get_dvar_string(const std::string& dvar)
		{
			auto* dvar_value = game::Dvar_FindVar(dvar.data());
			if (dvar_value && dvar_value->current.string)
			{
				return dvar_value->current.string;
			}

			return {};
		}

		int get_dvar_int(const std::string& dvar)
		{
			auto* dvar_value = game::Dvar_FindVar(dvar.data());
			if (dvar_value && dvar_value->current.integer)
			{
				return dvar_value->current.integer;
			}

			return -1;
		}

		bool get_dvar_bool(const std::string& dvar)
		{
			auto* dvar_value = game::Dvar_FindVar(dvar.data());
			if (dvar_value && dvar_value->current.enabled)
			{
				return dvar_value->current.enabled;
			}

			return false;
		}

		void didyouknow_stub(const char* dvar_name, const char* string)
		{
			if (!party::sv_motd.empty())
			{
				string = party::sv_motd.data();
			}

			// This function either does Dvar_SetString or Dvar_RegisterString for the given dvar
			reinterpret_cast<void(*)(const char*, const char*)>(0x1404C39B0)(dvar_name, string);
			party::sv_motd.clear();
		}
	}

	int get_client_count()
	{
		auto count = 0;
		for (auto i = 0; i < *game::mp::svs_numclients; ++i)
		{
			if (game::mp::svs_clients[i].header.state >= 3)
			{
				++count;
			}
		}

		return count;
	}

	int get_bot_count()
	{
		auto count = 0;
		for (auto i = 0; i < *game::mp::svs_numclients; ++i)
		{
			if (game::mp::svs_clients[i].header.state >= 3 &&
				game::SV_BotIsBot(i))
			{
				++count;
			}
		}

		return count;
	}

	void connect(const game::netadr_s& target)
	{
		if (game::environment::is_sp())
		{
			return;
		}

		command::execute("lui_open popup_acceptinginvite", false);

		connect_state.host = target;
		connect_state.challenge = utils::cryptography::random::get_challenge();

		network::send(target, "getInfo", connect_state.challenge);
	}

	void start_map(const std::string& mapname)
	{
		if (game::Live_SyncOnlineDataFlags(0) != 0)
		{
			scheduler::on_game_initialized([mapname]()
			{
				command::execute("map " + mapname, false);
			}, scheduler::pipeline::main, 1s);
		}
		else
		{
			if (!game::environment::is_dedi())
			{
				perform_game_initialization();
			}

			if (!game::SV_MapExists(mapname.data()))
			{
				printf("Map '%s' doesn't exist.", mapname.data());
				return;
			}

			auto* current_mapname = game::Dvar_FindVar("mapname");
			if (current_mapname && utils::string::to_lower(current_mapname->current.string) == utils::string::to_lower(mapname) && game::SV_Loaded())
			{
				printf("Restarting map: %s\n", mapname.data());
				command::execute("map_restart", false);
				return;
			}

			// starting map like this crashes the game.
			printf("Starting map: %s\n", mapname.data());
			game::SV_StartMapForParty(0, mapname.data(), true);
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

			command::add("map", [](const command::params& argument)
			{
				if (argument.size() != 2)
				{
					return;
				}

				start_map(argument[1]);
			});

			command::add("map_restart", []()
			{
				if (!game::SV_Loaded())
				{
					return;
				}
				*reinterpret_cast<int*>(0x1488692B0) = 1; // sv_map_restart
				*reinterpret_cast<int*>(0x1488692B4) = 1; // sv_loadScripts
				*reinterpret_cast<int*>(0x1488692B8) = 0; // sv_migrate
				reinterpret_cast<void(*)(int)>(0x140437460)(0); // SV_CheckLoadGame
			});

			command::add("fast_restart", []()
			{
				if (game::SV_Loaded())
				{
					game::SV_FastRestart(0);
				}
			});

			command::add("connect", [](const command::params& argument)
			{
				if (argument.size() != 2)
				{
					return;
				}

				game::netadr_s target{};
				if (game::NET_StringToAdr(argument[1], &target))
				{
					connect(target);
				}
			});

			command::add("reconnect", [](const command::params& params)
			{
				if (!game::CL_IsCgameInitialized())
				{
					return;
				}
				const auto host = connect_state.host;
				if (host.type == game::NA_LOOPBACK)
				{
					command::execute("map_restart", true);
					return;
				}
				if (host.type != game::NA_IP)
				{
					return;
				}
				command::execute("disconnect", true);
				scheduler::once([host]()
				{
					connect(host);
				}, scheduler::pipeline::async, 2s);
			});

			command::add("clientkick", [](const command::params& params)
			{
				if (params.size() < 2)
				{
					printf("usage: clientkick <num>\n");
					return;
				}
				const auto client_num = atoi(params.get(1));
				if (client_num < 0 || client_num >= *game::mp::svs_numclients)
				{
					return;
				}

				game::SV_KickClientNum(client_num, "EXE_PLAYERKICKED");
			});

			scheduler::once([]()
			{
				game::Dvar_RegisterString("sv_sayName", "console", game::DvarFlags::DVAR_FLAG_NONE,
				                          "The name to pose as for 'say' commands");
			}, scheduler::pipeline::main);

			command::add("tell", [](const command::params& params)
			{
				if (params.size() < 3)
				{
					return;
				}

				const auto client_num = atoi(params.get(1));
				const auto message = params.join(2);
				const auto* const name = game::Dvar_FindVar("sv_sayName")->current.string;

				game::SV_GameSendServerCommand(client_num, 0,
				                               utils::string::va("%c \"%s: %s\"", 84, name, message.data()));
				printf("%s -> %i: %s\n", name, client_num, message.data());
			});

			command::add("tellraw", [](const command::params& params)
			{
				if (params.size() < 3)
				{
					return;
				}

				const auto client_num = atoi(params.get(1));
				const auto message = params.join(2);

				game::SV_GameSendServerCommand(client_num, 0, utils::string::va("%c \"%s\"", 84, message.data()));
				printf("%i: %s\n", client_num, message.data());
			});

			command::add("say", [](const command::params& params)
			{
				if (params.size() < 2)
				{
					return;
				}

				const auto message = params.join(1);
				const auto* const name = game::Dvar_FindVar("sv_sayName")->current.string;

				game::SV_GameSendServerCommand(
					-1, 0, utils::string::va("%c \"%s: %s\"", 84, name, message.data()));
				printf("%s: %s\n", name, message.data());
			});

			command::add("sayraw", [](const command::params& params)
			{
				if (params.size() < 2)
				{
					return;
				}

				const auto message = params.join(1);

				game::SV_GameSendServerCommand(-1, 0, utils::string::va("%c \"%s\"", 84, message.data()));
				printf("%s\n", message.data());
			});

			utils::hook::call(0x14048811C, didyouknow_stub); // allow custom didyouknow based on sv_motd

			network::on("getInfo", [](const game::netadr_s& target, const std::string_view& data)
			{
				utils::info_string info{};
				info.set("challenge", std::string{data});
				info.set("gamename", "S1");
				info.set("hostname", get_dvar_string("sv_hostname"));
				info.set("gametype", get_dvar_string("g_gametype"));
				info.set("sv_motd", get_dvar_string("sv_motd"));
				info.set("xuid", utils::string::va("%llX", steam::SteamUser()->GetSteamID().bits));
				info.set("mapname", get_dvar_string("mapname"));
				info.set("isPrivate", get_dvar_string("g_password").empty() ? "0" : "1");
				info.set("clients", utils::string::va("%i", get_client_count()));
				info.set("bots", utils::string::va("%i", get_bot_count()));
				info.set("sv_maxclients", utils::string::va("%i", *game::mp::svs_numclients));
				info.set("protocol", utils::string::va("%i", PROTOCOL));
				info.set("playmode", utils::string::va("%i", game::Com_GetCurrentCoDPlayMode()));

				network::send(target, "infoResponse", info.build(), '\n');
			});

			network::on("infoResponse", [](const game::netadr_s& target, const std::string_view& data)
			{
				const utils::info_string info{data};
				//server_list::handle_info_response(target, info);

				if (connect_state.host != target)
				{
					return;
				}

				if (info.get("challenge") != connect_state.challenge)
				{
					const auto str = "Invalid challenge.";
					printf("%s\n", str);
					game::Com_Error(game::ERR_DROP, str);
					return;
				}

				const auto gamename = info.get("gamename");
				if (gamename != "S1"s)
				{
					const auto str = "Invalid gamename.";
					printf("%s\n", str);
					game::Com_Error(game::ERR_DROP, str);
					return;
				}

				const auto playmode = info.get("playmode");
				if (game::CodPlayMode(std::atoi(playmode.data())) != game::Com_GetCurrentCoDPlayMode())
				{
					const auto str = "Invalid playmode.";
					printf("%s\n", str);
					game::Com_Error(game::ERR_DROP, str);
					return;
				}

				const auto mapname = info.get("mapname");
				if (mapname.empty())
				{
					const auto str = "Invalid map.";
					printf("%s\n", str);
					game::Com_Error(game::ERR_DROP, str);
					return;
				}

				const auto gametype = info.get("gametype");
				if (gametype.empty())
				{
					const auto str = "Invalid gametype.";
					printf("%s\n", str);
					game::Com_Error(game::ERR_DROP, str);
					return;
				}

				party::sv_motd = info.get("sv_motd");

				connect_to_party(target, mapname, gametype);
			});
		}
	};
}

REGISTER_COMPONENT(party::component)
