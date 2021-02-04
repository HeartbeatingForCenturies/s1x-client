#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdProfiles::bdProfiles() : service(8, "bdProfiles")
	{
		this->register_task(3, "unk3", &bdProfiles::unk3);
	}

	void bdProfiles::unk3(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
