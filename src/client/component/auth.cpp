#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "auth.hpp"
#include "command.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/info_string.hpp>
#include <utils/cryptography.hpp>

#include "game/game.hpp"

namespace auth
{
	namespace
	{
		std::string get_key_entropy()
		{
			HW_PROFILE_INFO info;
			if (!GetCurrentHwProfileA(&info))
			{
				utils::cryptography::random::get_challenge();
			}

			return info.szHwProfileGuid;
		}
	
		utils::cryptography::ecc::key& get_key()
		{
			static auto key = utils::cryptography::ecc::generate_key(512, get_key_entropy());
			return key;
		}
	}

	uint64_t get_guid()
	{
		if (game::environment::is_dedi())
		{
			return 0x110000100000000 | (::utils::cryptography::random::get_integer() & ~0x80000000);
		}

		return get_key().get_hash();
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			// Patch steam id bit check
			if (game::environment::is_sp())
			{
				utils::hook::jump(0x1404267F0, 0x140426846);
				utils::hook::jump(0x14042760F, 0x140427650);
				utils::hook::jump(0x140427AB4, 0x140427B02);
			}
			else
			{
				utils::hook::jump(0x140538920, 0x140538976);
				utils::hook::jump(0x140009801, 0x140009B48);
				utils::hook::jump(0x140009AEB, 0x140009B48);
				utils::hook::jump(0x14053995F, 0x1405399A0);
				utils::hook::jump(0x140539E70, 0x140539EB6);
			}
		}
	};
}

REGISTER_COMPONENT(auth::component)
