#pragma once

namespace demonware
{

	class bdRichPresence final : public service
	{
	public:
		bdRichPresence();

	private:
		void unk1(service_server* server, uint8_t type, byte_buffer* buffer) const;
		void unk2(service_server* server, uint8_t type, byte_buffer* buffer) const;
	};

}
