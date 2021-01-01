#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "command.hpp"
#include "game_console.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/memory.hpp>

namespace command
{
	namespace
	{
		utils::hook::detour client_command_hook;

		std::unordered_map<std::string, std::function<void(params&)>> handlers;
		std::unordered_map<std::string, std::function<void(int, params_sv&)>> handlers_sv;

		void main_handler()
		{
			params params = {};

			const auto command = utils::string::to_lower(params[0]);
			if (handlers.find(command) != handlers.end())
			{
				handlers[command](params);
			}
		}

		void client_command(const int clientNum, void* a2)
		{
			params_sv params = {};

			const auto command = utils::string::to_lower(params[0]);
			if (handlers_sv.find(command) != handlers_sv.end())
			{
				handlers_sv[command](clientNum, params);
			}

			client_command_hook.invoke<void>(clientNum, a2);
		}

		// Shamelessly stolen from Quake3
		// https://github.com/id-Software/Quake-III-Arena/blob/dbe4ddb10315479fc00086f08e25d968b4b43c49/code/qcommon/common.c#L364
		void parse_command_line()
		{
			static auto parsed = false;
			if (parsed)
			{
				return;
			}

			static std::string comand_line_buffer = GetCommandLineA();
			char* command_line = comand_line_buffer.data();

			auto& com_num_console_lines = *reinterpret_cast<int*>(0x141AE732C);
			auto* com_console_lines = reinterpret_cast<char**>(0x141AE7330);

			auto inq = false;
			com_console_lines[0] = command_line;
			com_num_console_lines = 0;

			while (*command_line)
			{
				if (*command_line == '"')
				{
					inq = !inq;
				}
				// look for a + separating character
				// if commandLine came from a file, we might have real line seperators
				if ((*command_line == '+' && !inq) || *command_line == '\n' || *command_line == '\r')
				{
					if (com_num_console_lines == 0x20) // MAX_CONSOLE_LINES
					{
						break;
					}
					com_console_lines[com_num_console_lines] = command_line + 1;
					com_num_console_lines++;
					*command_line = '\0';
				}
				command_line++;
			}
			parsed = true;
		}

		void parse_commandline_stub()
		{
			parse_command_line();
			reinterpret_cast<void(*)()>(0x1403CEE10)();
		}
	}

	void read_startup_variable(const std::string& dvar)
	{
		// parse the commandline if it's not parsed
		parse_command_line();

		auto& com_num_console_lines = *reinterpret_cast<int*>(0x141AE732C);
		auto* com_console_lines = reinterpret_cast<char**>(0x141AE7330);

		for (int i = 0; i < com_num_console_lines; i++)
		{
			game::Cmd_TokenizeString(com_console_lines[i]);

			// only +set dvar value
			if (game::Cmd_Argc() >= 3 && game::Cmd_Argv(0) == "set"s && game::Cmd_Argv(1) == dvar)
			{
				game::Dvar_SetCommand(game::Cmd_Argv(1), game::Cmd_Argv(2));
			}

			game::Cmd_EndTokenizeString();
		}
	}

	params::params()
		: nesting_(game::cmd_args->nesting)
	{
	}

	int params::size() const
	{
		return game::cmd_args->argc[this->nesting_];
	}

	const char* params::get(const int index) const
	{
		if (index >= this->size())
		{
			return "";
		}

		return game::cmd_args->argv[this->nesting_][index];
	}

	std::string params::join(const int index) const
	{
		std::string result = {};

		for (auto i = index; i < this->size(); i++)
		{
			if (i > index) result.append(" ");
			result.append(this->get(i));
		}
		return result;
	}

	params_sv::params_sv()
		: nesting_(game::sv_cmd_args->nesting)
	{
	}

	int params_sv::size() const
	{
		return game::sv_cmd_args->argc[this->nesting_];
	}

	const char* params_sv::get(const int index) const
	{
		if (index >= this->size())
		{
			return "";
		}

		return game::sv_cmd_args->argv[this->nesting_][index];
	}

	std::string params_sv::join(const int index) const
	{
		std::string result = {};

		for (auto i = index; i < this->size(); i++)
		{
			if (i > index) result.append(" ");
			result.append(this->get(i));
		}
		return result;
	}

	void add_raw(const char* name, void (*callback)())
	{
		game::Cmd_AddCommandInternal(name, callback, utils::memory::get_allocator()->allocate<game::cmd_function_s>());
	}

	void add(const char* name, const std::function<void(const params&)>& callback)
	{
		const auto command = utils::string::to_lower(name);

		if (handlers.find(command) == handlers.end())
			add_raw(name, main_handler);

		handlers[command] = callback;
	}

	void add(const char* name, const std::function<void()>& callback)
	{
		add(name, [callback](const params&)
		{
			callback();
		});
	}

	void add_sv(const char* name, std::function<void(int, const params_sv&)> callback)
	{
		// doing this so the sv command would show up in the console
		add_raw(name, nullptr);

		const auto command = utils::string::to_lower(name);

		if (handlers_sv.find(command) == handlers_sv.end())
			handlers_sv[command] = std::move(callback);
	}

	void execute(std::string command, const bool sync)
	{
		command += "\n";

		if (sync)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, command.data());
		}
		else
		{
			game::Cbuf_AddText(0, command.data());
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (game::environment::is_sp())
			{
				add_commands_sp();
			}
			else
			{
				utils::hook::call(0x1403CDF1C, &parse_commandline_stub);

				add_commands_mp();
			}
			add_commands_generic();
		}

	private:
		static void add_commands_generic()
		{
			add("quit", game::Com_Quit_f);
			add("hard_quit", utils::nt::raise_hard_exception);
			add("crash", []()
			{
				*reinterpret_cast<int*>(1) = 0;
			});

			add("print", [](const params& params)
			{
				auto msg = params.join(1);
				printf("%s\n", msg.data());
			});

			add("printError", [](const params& params)
			{
				auto msg = params.join(1);
				game::Com_Error(game::ERR_DROP, "%s\n", msg.data());
			});

			add("dvarDump", []()
			{
				game_console::print(game_console::con_type_info,
					"================================ DVAR DUMP ========================================\n");
				for (auto i = 0; i < *game::dvarCount; i++)
				{
					const auto dvar = game::sortedDvars[i];
					if (dvar)
					{
						game_console::print(game_console::con_type_info, "%s \"%s\"\n", dvar->name,
							game::Dvar_ValueToString(dvar, dvar->current));
					}
				}
				game_console::print(game_console::con_type_info, "\n%i dvar indexes\n", *game::dvarCount);
				game_console::print(game_console::con_type_info,
					"================================ END DVAR DUMP ====================================\n");
			});

			add("commandDump", []()
			{
				game_console::print(game_console::con_type_info,
					"================================ COMMAND DUMP =====================================\n");
				game::cmd_function_s* cmd = (*game::cmd_functions);
				int i = 0;
				while (cmd)
				{
					if (cmd->name)
					{
						game_console::print(game_console::con_type_info, "%s\n", cmd->name);
						i++;
					}
					cmd = cmd->next;
				}
				game_console::print(game_console::con_type_info, "\n%i command indexes\n", i);
				game_console::print(game_console::con_type_info,
					"================================ END COMMAND DUMP =================================\n");
			});
		}

		static void add_commands_sp()
		{
			
		}

		static void add_commands_mp()
		{
			client_command_hook.create(0x1402E98F0, &client_command);

			add("map", [](const command::params& params)
			{
				if (params.size() != 2)
				{
					return;
				}

				auto mapname = params.get(1);

				if (!game::SV_MapExists(mapname))
				{
					printf("Map '%s' doesn't exist.", mapname);
					return;
				}

				auto* current_mapname = game::Dvar_FindVar("mapname");
				if (current_mapname && utils::string::to_lower(current_mapname->current.string) == utils::string::to_lower(mapname) && game::SV_Loaded())
				{
					printf("Restarting map: %s\n", mapname);
					command::execute("map_restart", false);
					return;
				}

				printf("Starting map: %s\n", mapname);
				//game::SV_StartMap(0, mapname);
				game::SV_StartMapForParty(0, mapname, false);
			});

			add("map_restart", []()
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

			add("fast_restart", []()
			{
				if (game::SV_Loaded())
				{
					game::SV_FastRestart(0);
				}
			});

			add("clientkick", [](const command::params& params)
			{
				if (params.size() < 2)
				{
					printf("usage: clientkick <num>\n");
					return;
				}
				const auto client_num = atoi(params.get(1));
				if (client_num < 0 || client_num >= *game::svs_numclients)
				{
					return;
				}

				game::SV_KickClientNum(client_num, "EXE_PLAYERKICKED");
			});
		}
	};
}

REGISTER_COMPONENT(command::component)
