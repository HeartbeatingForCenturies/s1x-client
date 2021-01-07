#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "command.hpp"
#include "game_console.hpp"
#include "game/game.hpp"
#include "game/dvars.hpp"
#include "scheduler.hpp"
#include "dvars.hpp"
#include "fastfiles.hpp"

#include <utils/string.hpp>
#include <utils/hook.hpp>

namespace patches
{
	namespace
	{
		utils::hook::detour live_get_local_client_name_hook;

		const char* live_get_local_client_name()
		{
			return game::Dvar_FindVar("name")->current.string;
		}

		utils::hook::detour sv_kick_client_num_hook;

		void sv_kick_client_num(const int clientNum, const char* reason)
		{
			// Don't kick bot to equalize team balance.
			if (reason == "EXE_PLAYERKICKED_BOT_BALANCE"s)
			{
				return;
			}
			return sv_kick_client_num_hook.invoke<void>(clientNum, reason);
		}

		utils::hook::detour com_register_dvars_hook;

		void com_register_dvars()
		{
			if (game::environment::is_mp())
			{
				// Make name save
				game::Dvar_RegisterString("name", "Unknown Soldier", 0x1, "Player name.");

				// Disable data validation error popup
				game::Dvar_RegisterInt("data_validation_allow_drop", 0, 0, 0, 0, "");
			}

			return com_register_dvars_hook.invoke<void>();
		}

		game::dvar_t* register_com_maxfps_stub(const char* name, int /*value*/, int /*min*/, int /*max*/,
			const unsigned int /*flags*/,
			const char* description)
		{
			return game::Dvar_RegisterInt(name, 0, 0, 1000, 0x1, description);
		}

		game::dvar_t* register_cg_fov_stub(const char* name, float value, float min, float /*max*/,
			const unsigned int flags,
			const char* description)
		{
			return game::Dvar_RegisterFloat(name, value, min, 160, flags | 0x1, description);
		}

		game::dvar_t* register_fovscale_stub(const char* name, float /*value*/, float /*min*/, float /*max*/,
		                                     unsigned int /*flags*/,
		                                     const char* desc)
		{
			// changed max value from 2.0f -> 5.0f and min value from 0.5f -> 0.1f
			return game::Dvar_RegisterFloat(name, 1.0f, 0.1f, 5.0f, 0x1, desc);
		}

		int dvar_command_patch() // game makes this return an int and compares with eax instead of al -_-
		{
			const command::params args{};

			if (args.size() <= 0)
				return 0;

			auto* dvar = game::Dvar_FindVar(args.get(0));
			if (dvar)
			{
				if (args.size() == 1)
				{
					const auto current = game::Dvar_ValueToString(dvar, dvar->current);
					const auto reset = game::Dvar_ValueToString(dvar, dvar->reset);
					game_console::print(game_console::con_type_info, "\"%s\" is: \"%s^7\" default: \"%s^7\"",
					                    dvar->name, current, reset);
					game_console::print(game_console::con_type_info, "   %s\n",
					                    dvars::dvar_get_domain(dvar->type, dvar->domain).data());
				}
				else
				{
					char command[0x1000] = {0};
					game::Dvar_GetCombinedString(command, 1);
					game::Dvar_SetCommand(args.get(0), command);
				}

				return 1;
			}

			return 0;
		}

		void missing_content_error_stub(int /*mode*/, const char* /*message*/)
		{
			game::Com_Error(game::ERR_DROP, utils::string::va("MISSING FILE\n%s.ff", fastfiles::get_current_fastfile()));
		}

		void bsp_sys_error_stub(const char* error, const char* arg1)
		{
			if (game::environment::is_dedi())
			{
				game::Sys_Error(error, arg1);
			}
			else
			{
				game::Com_Error(game::ERR_DROP, error, arg1);
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			// Increment ref-count on these
			LoadLibraryA("PhysXDevice64.dll");
			LoadLibraryA("PhysXUpdateLoader64.dll");

			// Register dvars
			com_register_dvars_hook.create(SELECT_VALUE(0x1402F86F0, 0x1403CF7F0), &com_register_dvars);

			// Unlock fps in main menu
			utils::hook::set<BYTE>(SELECT_VALUE(0x140144F5B, 0x140213C3B), 0xEB);

			// Unlock fps
			utils::hook::call(SELECT_VALUE(0x1402F8726, 0x1403CF8CA), register_com_maxfps_stub);

			// Unlock cg_fov
			utils::hook::call(SELECT_VALUE(0x1400EF830, 0x140014F66), register_cg_fov_stub);

			// Unlock cg_fovscale
			utils::hook::call(SELECT_VALUE(0x140227599, 0x140014F9B), register_fovscale_stub);

			// Patch Dvar_Command to print out values how CoD4 does it
			utils::hook::jump(SELECT_VALUE(0x1402FB4C0, 0x1403D31C0), dvar_command_patch);

			// Show missing fastfiles
			utils::hook::call(SELECT_VALUE(0x1401817AF, 0x1402742A8), missing_content_error_stub);

			// Fix mouse lag
			utils::hook::nop(SELECT_VALUE(0x14038FAFF, 0x1404DB1AF), 6);
			scheduler::loop([]()
			{
				SetThreadExecutionState(ES_DISPLAY_REQUIRED);
			}, scheduler::pipeline::main);

			if (game::environment::is_sp())
			{
				patch_sp();
			}
			else
			{
				patch_mp();
			}
		}

		static void patch_mp()
		{
			// Use name dvar
			live_get_local_client_name_hook.create(0x1404D47F0, &live_get_local_client_name);

			// Patch SV_KickClientNum
			sv_kick_client_num_hook.create(0x1404377A0, &sv_kick_client_num);

			// patch "Couldn't find the bsp for this map." error to not be fatal in mp
			utils::hook::call(0x14026E63B, bsp_sys_error_stub);
		}

		static void patch_sp()
		{

		}
	};
}

REGISTER_COMPONENT(patches::component)
