#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdGroups::bdGroups() : service(28, "bdGroup")
	{
		this->register_task(1, "set_groups", &bdGroups::set_groups);
		this->register_task(4, "unk4", &bdGroups::unk4);
	}

	void bdGroups::set_groups(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdGroups::unk4(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
