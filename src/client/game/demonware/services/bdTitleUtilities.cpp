#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{
	bdTitleUtilities::bdTitleUtilities() : service(12, "bdTitleUtilities")
	{
		this->register_task(6, "get server time", &bdTitleUtilities::get_server_time);
	}

	void bdTitleUtilities::get_server_time(service_server* server, uint8_t type, byte_buffer* /*buffer*/) const
	{
		const auto time_result = new bdTimeStamp;
		time_result->unix_time = uint32_t(time(nullptr));

		auto reply = server->create_reply(type);
		reply->add(time_result);
		reply->send();
	}
}
