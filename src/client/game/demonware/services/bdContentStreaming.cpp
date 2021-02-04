#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdContentStreaming::bdContentStreaming() : service(50, "bdContentStreaming")
	{
		this->register_task(2, "unk2", &bdContentStreaming::unk2);
		this->register_task(3, "unk3", &bdContentStreaming::unk3);
	}

	void bdContentStreaming::unk2(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdContentStreaming::unk3(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
