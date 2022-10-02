#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>

#include "component/console.hpp"

#include <xsk/gsc/types.hpp>
#include <xsk/resolver.hpp>

namespace gsc
{
	namespace
	{
		typedef void(*builtin_function)();
		std::unordered_map<std::uint32_t, builtin_function> builtin_funcs_overrides;

		utils::hook::detour scr_register_function_hook;

		void override_function(const std::string& name, builtin_function func)
		{
			const auto id = xsk::gsc::s1::resolver::function_id(name);
			builtin_funcs_overrides.emplace(id, func);
		}

		void scr_register_function_stub(void* func, int type, unsigned int name)
		{
			if (const auto got = builtin_funcs_overrides.find(name); got != builtin_funcs_overrides.end())
			{
				func = got->second;
			}

			scr_register_function_hook.invoke<void>(func, type, name);
		}

		void scr_print()
		{
			for (auto i = 0u; i < game::Scr_GetNumParam(); ++i)
			{
				console::info("%s", game::Scr_GetString(i));
			}
		}

		void scr_print_ln()
		{
			for (auto i = 0u; i < game::Scr_GetNumParam(); ++i)
			{
				console::info("%s", game::Scr_GetString(i));
			}

			console::info("\n");
		}
	}

	class extension final : public component_interface
	{
	public:
		void post_unpack() override
		{
			scr_register_function_hook.create(game::Scr_RegisterFunction, &scr_register_function_stub);

			override_function("print", &scr_print);
			override_function("println", &scr_print_ln);
		}
	};
}

REGISTER_COMPONENT(gsc::extension)
