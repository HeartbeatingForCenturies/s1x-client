#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdFacebook::bdFacebook() : service(36, "bdFacebook")
	{
		this->register_task(1, "unk1", &bdFacebook::unk1);
		this->register_task(3, "unk3", &bdFacebook::unk3);
		this->register_task(7, "unk7", &bdFacebook::unk7);
		this->register_task(8, "unk8", &bdFacebook::unk8);
	}

	void bdFacebook::unk1(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdFacebook::unk3(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdFacebook::unk7(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdFacebook::unk8(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
