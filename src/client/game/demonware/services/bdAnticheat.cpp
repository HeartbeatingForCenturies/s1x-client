#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdAnticheat::bdAnticheat() : service(38, "bdAnticheat")
	{
		this->register_task(2, "unk2", &bdAnticheat::unk2);
		this->register_task(4, "report console details", &bdAnticheat::report_console_details);
	}

	void bdAnticheat::unk2(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO: Read data as soon as needed
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdAnticheat::report_console_details(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO: Read data as soon as needed
		auto reply = server->create_reply(type);
		reply->send();
	}

}
