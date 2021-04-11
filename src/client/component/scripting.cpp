#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include <utils/hook.hpp>

#include "game/scripting/entity.hpp"
#include "game/scripting/event.hpp"
#include "game/scripting/lua/engine.hpp"
#include "game/scripting/execution.hpp"

#include "scheduler.hpp"

namespace scripting
{
	namespace
	{
		utils::hook::detour vm_notify_hook;
		utils::hook::detour scr_load_level_hook;
		utils::hook::detour g_shutdown_game_hook;

		void vm_notify_stub(const unsigned int notify_list_owner_id, const game::scr_string_t string_value,
		                    game::VariableValue* top)
		{
			if (!game::VirtualLobby_Loaded())
			{
				const auto* string = game::SL_ConvertToString(string_value);
				if (string)
				{
					event e;
					e.name = string;
					e.entity = notify_list_owner_id;

					for (auto* value = top; value->type != game::SCRIPT_END; --value)
					{
						e.arguments.emplace_back(*value);
					}

					if (e.name == "connected")
					{
						scripting::clear_entity_fields(e.entity);
					}

					lua::engine::notify(e);
				}
			}

			vm_notify_hook.invoke<void>(notify_list_owner_id, string_value, top);
		}

		void scr_load_level_stub()
		{
			
			scr_load_level_hook.invoke<void>();
			if (!game::VirtualLobby_Loaded())
			{
				lua::engine::start();
			}
		}

		void g_shutdown_game_stub(const int free_scripts)
		{
			if (!game::VirtualLobby_Loaded())
			{
				lua::engine::stop();
			}
			return g_shutdown_game_hook.invoke<void>(free_scripts);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			vm_notify_hook.create(SELECT_VALUE(0x140320E50, 0x1403FD5B0), vm_notify_stub);
			// SP address is wrong, but should be ok
			scr_load_level_hook.create(SELECT_VALUE(0x140005260, 0x140325B90), scr_load_level_stub);
			g_shutdown_game_hook.create(SELECT_VALUE(0x140228BA0, 0x1402F8C10), g_shutdown_game_stub);

			scheduler::loop([]()
			{
				lua::engine::run_frame();
			}, scheduler::pipeline::server);
		}
	};
}

REGISTER_COMPONENT(scripting::component)
