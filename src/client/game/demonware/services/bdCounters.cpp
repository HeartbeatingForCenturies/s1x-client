#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdCounters::bdCounters() : service(23, "bdCounters")
	{
		this->register_task(1, "unk1", &bdCounters::unk1);
		this->register_task(2, "unk2", &bdCounters::unk2);
	}

	void bdCounters::unk1(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdCounters::unk2(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
