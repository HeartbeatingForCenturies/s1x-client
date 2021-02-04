#pragma once

namespace demonware
{

	class bdContentStreaming final : public service
	{
	public:
		bdContentStreaming();

	private:
		void unk2(service_server* server, uint8_t type, byte_buffer* buffer) const;
		void unk3(service_server* server, uint8_t type, byte_buffer* buffer) const;
	};

}
