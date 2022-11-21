#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "dvars.hpp"

#include "game/game.hpp"
#include "game/dvars.hpp"

#include <utils/hook.hpp>
#include <utils/flags.hpp>

namespace ranked
{
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (game::environment::is_sp())
			{
				return;
			}

			if (game::environment::is_mp())
			{
				dvars::override::register_bool("xblive_privatematch", false, game::DVAR_FLAG_REPLICATED);
			}

			if (game::environment::is_dedi() && !utils::flags::has_flag("unranked"))
			{
				dvars::override::register_bool("xblive_privatematch", false, game::DVAR_FLAG_REPLICATED | game::DVAR_FLAG_WRITE);

				// Some dvar used in gsc
				game::Dvar_RegisterBool("force_ranking", true, game::DVAR_FLAG_WRITE, "Force ranking");
			}

			// Always run bots, even if xblive_privatematch is 0
			utils::hook::set(0x14013A0C0, 0xC301B0); // BG_BotSystemEnabled
			utils::hook::set(0x140139A60, 0xC301B0); // BG_AISystemEnabled
			utils::hook::set(0x14013A080, 0xC301B0); // BG_BotFastFileEnabled
			utils::hook::set(0x14013A200, 0xC301B0); // BG_BotsUsingTeamDifficulty
		}
	};
}

REGISTER_COMPONENT(ranked::component)
