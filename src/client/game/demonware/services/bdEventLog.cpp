#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdEventLog::bdEventLog() : service(67, "bdEventLog")
	{
		this->register_task(6, "unk6", &bdEventLog::unk6);
	}

	void bdEventLog::unk6(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
