#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{
	bdMarketing::bdMarketing() : service(139, "bdMarketing")
	{
		this->register_task(3, &bdMarketing::unk3);
	}

	void bdMarketing::unk3(service_server* server, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}
}
