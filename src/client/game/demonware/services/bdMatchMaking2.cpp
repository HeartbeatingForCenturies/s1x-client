#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdMatchMaking2::bdMatchMaking2() : service(138, "bdMatchMaking2")
	{
		this->register_task(1, "unk1", &bdMatchMaking2::unk1);
		this->register_task(2, "unk2", &bdMatchMaking2::unk2);
		this->register_task(3, "unk3", &bdMatchMaking2::unk3);
		this->register_task(5, "unk5", &bdMatchMaking2::unk5);
		this->register_task(16, "unk16", &bdMatchMaking2::unk16);
	}

	void bdMatchMaking2::unk1(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdMatchMaking2::unk2(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdMatchMaking2::unk3(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdMatchMaking2::unk5(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdMatchMaking2::unk16(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
