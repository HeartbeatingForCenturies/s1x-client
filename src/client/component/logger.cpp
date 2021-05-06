#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "console.hpp"

#include <utils/hook.hpp>

namespace logger
{
	namespace
	{
		utils::hook::detour com_error_hook;

		void print_error(const char* msg, ...)
		{
			char buffer[2048];

			va_list ap;
			va_start(ap, msg);

			vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, msg, ap);

			va_end(ap);

			console::error(buffer);
		}

		void print_com_error(int, const char* msg, ...)
		{
			char buffer[2048];

			va_list ap;
			va_start(ap, msg);

			vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, msg, ap);

			va_end(ap);

			console::error(buffer);
		}

		void com_error_stub(const int error, const char* msg, ...)
		{
			char buffer[2048];

			{
				va_list ap;
				va_start(ap, msg);

				vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, msg, ap);

				va_end(ap);

				console::error("Error: %s\n", buffer);
			}

			com_error_hook.invoke<void>(error, "%s", buffer);
		}

		void print_warning(const char* msg, ...)
		{
			char buffer[2048];

			va_list ap;
			va_start(ap, msg);

			vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, msg, ap);

			va_end(ap);

			console::warn(buffer);
		}

		void print(const char* msg, ...)
		{
			char buffer[2048];

			va_list ap;
			va_start(ap, msg);

			vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, msg, ap);

			va_end(ap);

			console::info(buffer);
		}

		void print_dev(const char* msg, ...)
		{
			static auto* enabled =
				game::Dvar_RegisterBool("logger_dev", false, game::DVAR_FLAG_SAVED, "Print dev stuff");
			if (!enabled->current.enabled)
			{
				return;
			}

			char buffer[2048];

			va_list ap;
			va_start(ap, msg);

			vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, msg, ap);

			va_end(ap);

			console::info(buffer);
		}

		// nullsub_56
		void nullsub_56()
		{
			utils::hook::call(0x1400D2572, print_warning);
			utils::hook::call(0x1400D257E, print_warning);
			utils::hook::call(0x1400D2586, print_warning);
			utils::hook::call(0x1400D2592, print_warning);

			utils::hook::call(0x1400D2B7D, print_warning);
			utils::hook::call(0x1400D2B89, print_warning);
			utils::hook::call(0x1400D2B91, print_warning);
			utils::hook::call(0x1400D2B9D, print_warning);

			utils::hook::call(0x1400D78ED, print_warning);
			utils::hook::call(0x1400D78F9, print_warning);
			utils::hook::call(0x1400D7901, print_warning);
			utils::hook::call(0x1400D790D, print_warning);

			utils::hook::call(0x1400D84F8, print_warning);
			utils::hook::call(0x1400D850C, print_warning);

			utils::hook::call(0x1400D8C08, print_warning);
			utils::hook::call(0x1400D8C28, print_warning);
			utils::hook::call(0x1400D8C34, print_warning);

			utils::hook::call(0x1400D8CD8, print_warning);
			utils::hook::call(0x1400D8CF8, print_warning);
			utils::hook::call(0x1400D8D04, print_warning);
			utils::hook::call(0x1400D8D1D, print_warning);

			utils::hook::call(0x1400DAE67, print_warning);
			utils::hook::call(0x1400DB019, print_warning);
		}

		// sub_1400E7420
		void sub_1400E7420()
		{
			utils::hook::call(0x1400CEC1C, print_warning);
			utils::hook::call(0x1400D218E, print_warning);
			utils::hook::call(0x1400D2319, print_warning);
			utils::hook::call(0x1400D65BC, print_dev);
			utils::hook::call(0x1400D7C94, print_dev);
			utils::hook::call(0x1400DAB6A, print_dev);
			utils::hook::call(0x1400DB9BC, print_dev);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (game::environment::is_mp())
			{
				nullsub_56();
				sub_1400E7420();
			}

			if (!game::environment::is_sp())
			{
				utils::hook::call(0x1404D8543, print_com_error);
			}

			com_error_hook.create(game::Com_Error, com_error_stub);
		}
	};
}

REGISTER_COMPONENT(logger::component)
