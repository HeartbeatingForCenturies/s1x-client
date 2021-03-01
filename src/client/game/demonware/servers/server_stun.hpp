#pragma once

#include "new/udp_server.hpp"

namespace demonware
{
	class server_stun : public udp_server
	{
	public:
		using udp_server::udp_server;

	private:
		void handle(const std::string& packet) override
		{
			uint8_t type, version, padding;

			byte_buffer buffer(packet);
			buffer.set_use_data_types(false);
			buffer.read_byte(&type);
			buffer.read_byte(&version);
			buffer.read_byte(&padding);

			switch (type)
			{
			case 30:
				this->ip_discovery();
				break;
			case 20:
				this->nat_discovery();
				break;
			default:
				break;
			}
		}

		void ip_discovery()
		{
			const uint32_t ip = 0x0100007f;

			byte_buffer buffer;
			buffer.set_use_data_types(false);
			buffer.write_byte(31); // type
			buffer.write_byte(2); // version
			buffer.write_byte(0); // version
			buffer.write_uint32(ip); // external ip
			buffer.write_uint16(3074); // port

			this->send(buffer.get_buffer());
		}

		void nat_discovery()
		{
			const uint32_t ip = 0x0100007f;

			byte_buffer buffer;
			buffer.set_use_data_types(false);
			buffer.write_byte(21); // type
			buffer.write_byte(2); // version
			buffer.write_byte(0); // version
			buffer.write_uint32(ip); // external ip
			buffer.write_uint16(3074); // port
			buffer.write_uint32(this->get_address()); // server ip
			buffer.write_uint16(3074); // server port

			this->send(buffer.get_buffer());
		}
	};
}
