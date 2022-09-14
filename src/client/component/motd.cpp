#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "motd.hpp"
#include "images.hpp"

#include <utils/hook.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>

#include <resource.hpp>

#define FEAUTURED_TEMPLATE "{\"content_short\": \"Welcome to S1X\", \"image\": \"icon_pl_cat_exo\", \"content_long\": \"<body>After Years of gaining experience towards reverse engineering, xLabs developers written a fresh SDK for cod game series from scratch; a new client base which made it possible to push forward and modify x64 generation cod games. iw6x offers variety of new features aswell as unlocking access to developer features like dedicated server hosting and game console<br/><br/><i>*visit xlabs.dev for more info</i></body>\", \"popup_image\": \"iotd_image\", \"action\": \"popup\"}"

namespace motd
{
	namespace
	{
		std::string motd_resource = utils::nt::load_resource(DW_MOTD);
		std::future<std::optional<std::string>> motd_future;
		std::string marketing_featured_msg_str;
	}

	std::string get_text()
	{
		try
		{
			return motd_future.get().value_or(motd_resource);
		}
		catch (std::exception&)
		{
		}

		return motd_resource;
	}

	utils::hook::detour marketing_get_message_hook;

	bool marketing_get_message_stub(int controllerIndex, int locationID, char* messageText, int messageTextLength)
	{
		if(marketing_featured_msg_str.empty()) return false;

		strncpy(messageText, marketing_featured_msg_str.data(), messageTextLength);

		return true;
	}
	
	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			motd_future = utils::http::get_data_async("https://xlabs.dev/s1/motd.txt");
			std::thread([]()
			{
				auto data = utils::http::get_data("https://xlabs.dev/s1/motd.png");
				if(data)
				{
					images::override_texture("iotd_image", data.value());
				}
				
				auto featured_optional = utils::http::get_data("https://xlabs.dev/s1/featured.json");
				if (featured_optional)
				{
					marketing_featured_msg_str = featured_optional.value();
				}
				else
				{
					marketing_featured_msg_str = FEAUTURED_TEMPLATE; // FOR PREVIEW PURPOSE; REMOVE THIS AFTER ADDING CONTENT TO WEBSITE DEPOT
				}
				
			}).detach();
		}
		
		void post_unpack() override
		{
			marketing_get_message_hook.create(0x140126930, marketing_get_message_stub); // not sure why but in s1x, client doesnt ask for maketing messages from demonware even with marketing_active set to true
		}
	};
}

REGISTER_COMPONENT(motd::component)
