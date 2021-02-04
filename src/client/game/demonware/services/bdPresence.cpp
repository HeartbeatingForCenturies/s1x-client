#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdPresence::bdPresence() : service(103, "bdPresence")
	{
		this->register_task(1, "unk1", &bdPresence::unk1);
		this->register_task(3, "unk3", &bdPresence::unk3);
	}

	void bdPresence::unk1(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdPresence::unk3(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
