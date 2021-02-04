#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{
	bdUNK104::bdUNK104() : service(104, "bdUNK104")
	{
		this->register_task(1, &bdUNK104::unk1);
	}

	void bdUNK104::unk1(service_server* server, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}
}
