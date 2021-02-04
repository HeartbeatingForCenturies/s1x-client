#pragma once

namespace demonware
{

	class bdStats final : public service
	{
	public:
		bdStats();

	private:
		void unk1(service_server* server, uint8_t type, byte_buffer* buffer) const;
		void unk3(service_server* server, uint8_t type, byte_buffer* buffer) const;
		void unk4(service_server* server, uint8_t type, byte_buffer* buffer) const;
		void unk8(service_server* server, uint8_t type, byte_buffer* buffer) const;
		void unk11(service_server* server, uint8_t type, byte_buffer* buffer) const;
	};

}
