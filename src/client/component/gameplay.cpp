#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/dvars.hpp"

#include <utils/hook.hpp>

namespace gameplay
{
	namespace
	{
		void stuck_in_client_stub(void* entity)
		{
			if (dvars::g_playerEjection->current.enabled)
			{
				reinterpret_cast<void(*)(void*)>(0x1402DA310)(entity); // StuckInClient
			}
		}

		void cm_transformed_capsule_trace_stub(struct trace_t* results, const float* start, const float* end,
			struct Bounds* bounds, struct Bounds* capsule, int contents, const float* origin, const float* angles)
		{
			if (dvars::g_playerCollision->current.enabled)
			{
				reinterpret_cast<void(*)
					(struct trace_t*, const float*, const float*, struct Bounds*, struct Bounds*, unsigned int, const float*, const float*)>
					(0x1403AB1C0)
					(results, start, end, bounds, capsule, contents, origin, angles); // CM_TransformedCapsuleTrace
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (game::environment::is_sp()) return;

			// Implement player ejection dvar
			dvars::g_playerEjection = game::Dvar_RegisterBool("g_playerEjection", true, game::DVAR_FLAG_REPLICATED, "Flag whether player ejection is on or off");
			utils::hook::call(0x1402D5E4A, stuck_in_client_stub);

			// Implement player collision dvar
			dvars::g_playerCollision = game::Dvar_RegisterBool("g_playerCollision", true, game::DVAR_FLAG_REPLICATED, "Flag whether player collision is on or off");
			utils::hook::call(0x1404563DA, cm_transformed_capsule_trace_stub); // SV_ClipMoveToEntity
			utils::hook::call(0x1401F7F8F, cm_transformed_capsule_trace_stub); // CG_ClipMoveToEntity
		}
	};
}

REGISTER_COMPONENT(gameplay::component)
