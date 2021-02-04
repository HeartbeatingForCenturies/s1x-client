#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdStats::bdStats() : service(4, "bdStats")
	{
		this->register_task(1, "unk1", &bdStats::unk1);
		this->register_task(3, "unk3", &bdStats::unk3); // leaderboards
		this->register_task(4, "unk4", &bdStats::unk4);
		this->register_task(8, "unk8", &bdStats::unk8);
		this->register_task(11, "unk11", &bdStats::unk11);
	}

	void bdStats::unk1(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdStats::unk3(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdStats::unk4(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdStats::unk8(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdStats::unk11(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
