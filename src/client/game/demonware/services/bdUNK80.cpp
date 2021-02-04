#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdUNK80::bdUNK80() : service(80, "bdUNK80")
	{
		this->register_task(42, "unk42", &bdUNK80::unk42);
		this->register_task(49, "unk49", &bdUNK80::unk49);
		this->register_task(60, "unk60", &bdUNK80::unk60);
		this->register_task(130, "unk130", &bdUNK80::unk130);
		this->register_task(165, "unk165", &bdUNK80::unk165);
		this->register_task(193, "unk193", &bdUNK80::unk193);
	}

	void bdUNK80::unk42(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdUNK80::unk49(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdUNK80::unk60(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdUNK80::unk130(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdUNK80::unk165(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdUNK80::unk193(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
