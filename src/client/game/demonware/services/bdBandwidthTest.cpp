#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	bdBandwidthTest::bdBandwidthTest() : service(18, "bdBandwidthTest")
	{
		this->register_task(204, "unk204", &bdBandwidthTest::unk204);
		this->register_task(2, "unk2", &bdBandwidthTest::unk2);
	}

	void bdBandwidthTest::unk204(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO: Read data as soon as needed
		auto reply = server->create_reply(type);
		reply->send();
	}

	void bdBandwidthTest::unk2(service_server* server, uint8_t type, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(type);
		reply->send();
	}

}
