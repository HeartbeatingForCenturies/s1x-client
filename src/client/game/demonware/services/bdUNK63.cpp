#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdUNK63::bdUNK63() : service(63, "bdUNK63")
	{
		//this->register_task(6, "unk6", &bdUNK63::unk6);
	}

	void bdUNK63::unk(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
