#pragma once

namespace demonware
{

	class bdPresence final : public service
	{
	public:
		bdPresence();

	private:
		void unk1(service_server* server, uint8_t type, byte_buffer* buffer) const;
		void unk3(service_server* server, uint8_t type, byte_buffer* buffer) const;
	};

}
