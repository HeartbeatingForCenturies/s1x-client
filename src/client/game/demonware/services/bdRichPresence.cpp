#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdRichPresence::bdRichPresence() : service(68, "bdRichPresence")
	{
		this->register_task(1, "unk1", &bdRichPresence::unk1);
		this->register_task(2, "unk2", &bdRichPresence::unk2);
	}

	void bdRichPresence::unk1(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdRichPresence::unk2(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
