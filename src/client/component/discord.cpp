#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"
#include "game/game.hpp"

#include <utils/string.hpp>

#include <discord_rpc.h>
#include <component/party.hpp>

namespace discord
{
	namespace
	{
		DiscordRichPresence discord_presence;

		void update_discord()
		{
			Discord_RunCallbacks();

			auto* dvar = game::Dvar_FindVar("virtualLobbyActive");
			if (!game::CL_IsCgameInitialized() || (dvar && dvar->current.enabled == 1))
			{
				discord_presence.details = game::environment::is_sp() ? "Singleplayer" : "Multiplayer";

				dvar = game::Dvar_FindVar("virtualLobbyInFiringRange");
				if (dvar && dvar->current.enabled == 1)
				{
					discord_presence.state = "Firing Range";
				}
				else
				{
					discord_presence.state = "Main Menu";
				}

				discord_presence.partySize = 0;
				discord_presence.partyMax = 0;
				discord_presence.startTimestamp = 0;

				discord_presence.largeImageKey = game::environment::is_sp() ? "menu_singleplayer" : "menu_multiplayer";
			}
			else
			{
				if (game::environment::is_sp()) return;

				const auto* gametype = game::UI_GetGameTypeDisplayName(game::Dvar_FindVar("ui_gametype")->current.string);
				const auto* map = game::UI_GetMapDisplayName(game::Dvar_FindVar("ui_mapname")->current.string);

				discord_presence.details = utils::string::va("%s on %s", gametype, map);

				auto* const host_name = reinterpret_cast<char*>(0x141646CC4);
				utils::string::strip(host_name, host_name, static_cast<int>(strlen(host_name)) + 1);

				if (!strcmp(host_name, "key"))
				{
					discord_presence.state = game::Dvar_FindVar("sv_hostname")->current.string;
				}
				else 
				{
					discord_presence.state = host_name;
				}

				dvar = game::Dvar_FindVar("sv_maxclients");
				if (dvar)
				{
					auto clients = reinterpret_cast<int*>(0x1414CC290);
					int clientsNum = *clients;
					discord_presence.partySize = clientsNum;
					discord_presence.partyMax = dvar->current.integer;
				}

				if (!discord_presence.startTimestamp)
				{
					discord_presence.startTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
						std::chrono::system_clock::now().time_since_epoch()).count();
				}

				discord_presence.largeImageKey = game::Dvar_FindVar("ui_mapname")->current.string;
				discord_presence.largeImageText = game::UI_GetGameTypeDisplayName(game::Dvar_FindVar("ui_mapname")->current.string);
			}

			Discord_UpdatePresence(&discord_presence);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			if (game::environment::is_dedi())
			{
				return;
			}

			DiscordEventHandlers handlers;
			ZeroMemory(&handlers, sizeof(handlers));
			handlers.ready = ready;
			handlers.errored = errored;
			handlers.disconnected = errored;
			handlers.joinGame = nullptr;
			handlers.spectateGame = nullptr;
			handlers.joinRequest = nullptr;

			Discord_Initialize("823223724013912124", &handlers, 1, nullptr);

			scheduler::once([]()
			{
				scheduler::once(update_discord, scheduler::pipeline::async);
				scheduler::loop(update_discord, scheduler::pipeline::async, 20s);
			}, scheduler::pipeline::main);

			initialized_ = true;
		}

		void pre_destroy() override
		{
			if (!initialized_ || game::environment::is_dedi())
			{
				return;
			}

			Discord_Shutdown();
		}

	private:
		bool initialized_ = false;

		static void ready(const DiscordUser* /*request*/)
		{
			ZeroMemory(&discord_presence, sizeof(discord_presence));

			discord_presence.instance = 1;

			Discord_UpdatePresence(&discord_presence);
		}

		static void errored(const int error_code, const char* message)
		{
			printf("Discord: (%i) %s", error_code, message);
		}
	};
}

#ifndef DEV_BUILD
REGISTER_COMPONENT(discord::component)
#endif
