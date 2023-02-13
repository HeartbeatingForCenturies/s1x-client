#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include "motd.hpp"
#include "images.hpp"

#include <utils/hook.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>

#include <resource.hpp>

namespace motd
{
	namespace
	{
		std::string motd_resource = utils::nt::load_resource(DW_MOTD);
		std::future<std::optional<std::string>> motd_future;
		std::string marketing_featured;
	}

	std::string get_text()
	{
		try
		{
			return motd_future.get().value_or(motd_resource);
		}
		catch (...)
		{
		}

		return motd_resource;
	}

	utils::hook::detour marketing_get_message_hook;

	bool marketing_get_message_stub(int /*controller_index*/, int /*location-id*/, char* message_text, int message_text_length)
	{
		if (marketing_featured.empty()) return false;

		strncpy_s(message_text, message_text_length, marketing_featured.data(), _TRUNCATE);

		return true;
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			motd_future = utils::http::get_data_async("https://xlabs.dev/s1/motd.txt");
			std::thread([]
			{
				auto data = utils::http::get_data("https://xlabs.dev/s1/motd.png");
				if (data.has_value())
				{
					images::override_texture("iotd_image", data.value());
				}

				auto featured_optional = utils::http::get_data("https://xlabs.dev/s1/featured_msg.json");
				if (featured_optional.has_value())
				{
					marketing_featured = featured_optional.value();
				}

			}).detach();
		}

		void post_unpack() override
		{
			if (game::environment::is_sp())
			{
				return;
			}

			// Not sure why but in S1x, client doesn't ask for maketing messages from demonware even though marketing_active set to true
			marketing_get_message_hook.create(0x140126930, marketing_get_message_stub);
		}

		void pre_destroy() override
		{
			marketing_get_message_hook.clear();
		}
	};
}

REGISTER_COMPONENT(motd::component)
