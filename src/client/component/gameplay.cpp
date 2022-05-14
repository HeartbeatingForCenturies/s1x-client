#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/dvars.hpp"

#include <utils/hook.hpp>

namespace gameplay
{
	namespace
	{
		utils::hook::detour pm_weapon_use_ammo_hook;

		int stuck_in_client_stub(game::mp::gentity_s* entity)
		{
			if (dvars::g_playerEjection->current.enabled)
			{
				return utils::hook::invoke<int>(0x1402DA310, entity); // StuckInClient
			}

			return 0;
		}

		void cm_transformed_capsule_trace_stub(game::trace_t* results, const float* start, const float* end,
			game::Bounds* bounds, game::Bounds* capsule, int contents, const float* origin, const float* angles)
		{
			if (dvars::g_playerCollision->current.enabled)
			{
				utils::hook::invoke<void>(0x1403AB1C0,
					results, start, end, bounds, capsule, contents, origin, angles); // CM_TransformedCapsuleTrace
			}
		}

		const auto pm_bouncing_stub_mp = utils::hook::assemble([](utils::hook::assembler& a)
		{
			const auto no_bounce = a.newLabel();
			const auto loc_14014DF48 = a.newLabel();

			a.push(rax);

			a.mov(rax, qword_ptr(reinterpret_cast<int64_t>(&dvars::pm_bouncing)));
			a.mov(al, byte_ptr(rax, 0x10));
			a.cmp(byte_ptr(rbp, -0x2D), al);

			a.pop(rax);
			a.jz(no_bounce);
			a.jmp(0x14014DFB0);

			a.bind(no_bounce);
			a.cmp(dword_ptr(rsp, 0x70), 0);
			a.jnz(loc_14014DF48);
			a.jmp(0x14014DFA2);

			a.bind(loc_14014DF48);
			a.jmp(0x14014DF48);
		});

		void pm_weapon_use_ammo_stub(game::playerState_s* ps, game::Weapon weapon,
			bool is_alternate, int amount, game::PlayerHandIndex hand)
		{
			if (!dvars::player_sustainAmmo->current.enabled)
			{
				pm_weapon_use_ammo_hook.invoke<void>(ps, weapon, is_alternate, amount, hand);
			}
		}

		const auto client_end_frame_stub = utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.push(rax);

			a.mov(rax, qword_ptr(reinterpret_cast<int64_t>(&dvars::g_gravity)));
			a.mov(eax, dword_ptr(rax, 0x10));
			a.mov(word_ptr(rbx, 0x36), ax);

			a.pop(rax);

			// Game code hook skipped
			a.mov(eax, dword_ptr(rbx, 0x5084));
			a.mov(rdi, rcx);

			a.jmp(0x1402D5A6A);
		});

		const auto client_think_real_stub = utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.push(rax);

			a.mov(rax, qword_ptr(reinterpret_cast<int64_t>(&dvars::g_speed)));
			a.mov(eax, dword_ptr(rax, 0x10));
			a.mov(word_ptr(rbx, 0x38), ax);

			a.pop(rax);

			// Game code hook skipped
			a.movzx(eax, word_ptr(rbx, 0x3C));
			a.add(eax, dword_ptr(rbx, 0x48));

			a.jmp(0x1402D6A9C);
		});

		const auto jump_push_off_ladder = utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.push(rax);

			a.mov(rax, qword_ptr(reinterpret_cast<int64_t>(&dvars::jump_ladderPushVel)));
			a.mulss(xmm7, dword_ptr(rax, 0x10));
			a.mulss(xmm6, dword_ptr(rax, 0x10));

			a.pop(rax);

			a.jmp(0x1401358C3);
		});

		void jump_start_stub(game::pmove_t* pm, game::pml_t* pml, float /*height*/)
		{
			utils::hook::invoke<void>(0x140135A90, pm, pml, dvars::jump_height->current.value);
		}

		void pm_player_trace_stub(game::pmove_t* pm, game::trace_t* results, const float* start,
			const float* end, const game::Bounds* bounds, int pass_entity_num, int content_mask)
		{
			utils::hook::invoke<void>(0x14014A420, pm, results, start, end, bounds, pass_entity_num, content_mask);

			if (dvars::g_elevators->current.enabled)
			{
				results->startsolid = false;
			}
		}

		void pm_trace_stub(const game::pmove_t* pm, game::trace_t* results, const float* start,
			const float* end, const game::Bounds* bounds, int pass_entity_num, int content_mask)
		{
			utils::hook::invoke<void>(0x14014A610, pm, results, start, end, bounds, pass_entity_num, content_mask);

			if (dvars::g_elevators->current.enabled)
			{
				results->allsolid = false;
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			dvars::player_sustainAmmo = game::Dvar_RegisterBool("player_sustainAmmo", false,
				game::DVAR_FLAG_REPLICATED, "Firing weapon will not decrease clip ammo");
			pm_weapon_use_ammo_hook.create(SELECT_VALUE(0x1403DD050, 0x140162B20), &pm_weapon_use_ammo_stub);

			if (game::environment::is_sp()) return;

			// Implement player ejection dvar
			dvars::g_playerEjection = game::Dvar_RegisterBool("g_playerEjection", true, game::DVAR_FLAG_REPLICATED,
			                                                  "Flag whether player ejection is on or off");
			utils::hook::call(0x1402D5E4A, stuck_in_client_stub);

			// Implement player collision dvar
			dvars::g_playerCollision = game::Dvar_RegisterBool("g_playerCollision", true, game::DVAR_FLAG_REPLICATED,
			                                                   "Flag whether player collision is on or off");
			utils::hook::call(0x1404563DA, cm_transformed_capsule_trace_stub); // SV_ClipMoveToEntity
			utils::hook::call(0x1401F7F8F, cm_transformed_capsule_trace_stub); // CG_ClipMoveToEntity

			// Implement bouncing dvar
			utils::hook::jump(0x14014DF91, pm_bouncing_stub_mp, true);
			dvars::pm_bouncing = game::Dvar_RegisterBool("pm_bouncing", false,
			                                             game::DVAR_FLAG_REPLICATED, "Enable bouncing");

			// Change jump_slowdownEnable dvar flags to just "replicated"
			utils::hook::set<uint8_t>(0x140135992, game::DVAR_FLAG_REPLICATED);

			// Choosing the following min/max because the game would truncate larger values
			dvars::g_gravity = game::Dvar_RegisterInt("g_gravity", 800, std::numeric_limits<short>::min(),
				std::numeric_limits<short>::max(), game::DVAR_FLAG_REPLICATED, "Gravity in inches per second per second");
			utils::hook::jump(0x1402D5A5D, client_end_frame_stub, true);
			utils::hook::nop(0x1402D5A69, 1); // Nop skipped opcode

			// Choosing the following min/max because the game would truncate larger values
			dvars::g_speed = game::Dvar_RegisterInt("g_speed", 190,
				std::numeric_limits<short>::min(), std::numeric_limits<short>::max(), game::DVAR_FLAG_REPLICATED, "Player speed");
			utils::hook::jump(0x1402D6A8C, client_think_real_stub, true);
			utils::hook::nop(0x1402D6A98, 4); // Nop skipped opcodes

			dvars::jump_height = game::Dvar_RegisterFloat("jump_height", 39.0f,
				0.0f, 1000.0f, game::DVAR_FLAG_REPLICATED, "The maximum height of a player's jump");
			utils::hook::call(0x1401352FF, jump_start_stub);

			dvars::jump_ladderPushVel = game::Dvar_RegisterFloat("jump_ladderPushVel", 128.0f,
				0.0f, 1024.0f, game::DVAR_FLAG_REPLICATED, "The velocity of a jump off of a ladder");
			utils::hook::jump(0x1401358B3, jump_push_off_ladder, true);
			utils::hook::nop(0x1401358BF, 4); // Nop skipped opcodes

			dvars::g_elevators = game::Dvar_RegisterBool("g_elevators", false,
				game::DVAR_FLAG_REPLICATED, "Enable elevators");
			utils::hook::call(0x140146134, pm_player_trace_stub);
			utils::hook::call(0x14014619B, pm_player_trace_stub);

			// Allow player to stand from prone/ducked
			utils::hook::call(0x140142AF6, pm_trace_stub);
			utils::hook::call(0x140142A1B, pm_trace_stub);
			utils::hook::call(0x14014298D, pm_trace_stub);
		}
	};
}

REGISTER_COMPONENT(gameplay::component)
