#include <std_include.hpp>
#include "game.hpp"

namespace game
{
	int Cmd_Argc()
	{
		return cmd_args->argc[cmd_args->nesting];
	}

	const char* Cmd_Argv(const int index)
	{
		return cmd_args->argv[cmd_args->nesting][index];
	}

	int SV_Cmd_Argc()
	{
		return sv_cmd_args->argc[sv_cmd_args->nesting];
	}

	const char* SV_Cmd_Argv(const int index)
	{
		return sv_cmd_args->argv[sv_cmd_args->nesting][index];
	}

	namespace environment
	{
		launcher::mode mode = launcher::mode::none;

		launcher::mode get_mode()
		{
			if (mode == launcher::mode::none)
			{
				throw std::runtime_error("Launcher mode not valid. Something must be wrong.");
			}

			return mode;
		}

		bool is_sp()
		{
			return get_mode() == launcher::mode::singleplayer;
		}

		bool is_mp()
		{
			return get_mode() == launcher::mode::multiplayer;
		}

		bool is_dedi()
		{
			return get_mode() == launcher::mode::server;
		}

		void set_mode(const launcher::mode _mode)
		{
			mode = _mode;
		}

		std::string get_string()
		{
			const auto current_mode = get_mode();
			switch (current_mode)
			{
			case launcher::mode::server:
				return "Dedicated Server";

			case launcher::mode::multiplayer:
				return "Multiplayer";

			case launcher::mode::singleplayer:
				return "Singleplayer";

			case launcher::mode::none:
				return "None";

			default:
				return "Unknown (" + std::to_string(static_cast<int>(mode)) + ")";
			}
		}

		std::string playmode_to_string(game::CodPlayMode playmode)
		{
			switch (playmode)
			{
			case CODPLAYMODE_CORE:
				return "Core";
			case CODPLAYMODE_ZOMBIES:
				return "Zombies";
			case CODPLAYMODE_SURVIVAL:
				return "Survival";
			case CODPLAYMODE_SP:
				return "Singleplayer";
			case CODPLAYMODE_NONE:
				return "None";
			default:
				return "Unknown (" + std::to_string(static_cast<int>(playmode)) + ")";
			}
		}
	}
}
